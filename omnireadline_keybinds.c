#include "omnifunc.h"
#include "omnireadline.h"

// Private Function To This File Only
static int omnish_bind_key(int key, rl_command_func_t *function);
static int omnish_bind_keyseq(const char *keyseq, rl_command_func_t *function);

// Tab Complete Functions
static int omnish_tab_complete(int count, int key);
static char **omnish_command_completion(const char *text, int start, int end);
static char *omnish_command_generator(const char *text, int state);

void omnish_init_keybinds(void)
{
    // Ask User To Display Results Above 100
    rl_completion_query_items = 80;
    // Keep Completion Results Alphabetical
    rl_sort_completion_matches = 1;
    // Print Duplicate Commands Found In Path Once
    rl_ignore_completion_duplicates = 1;
    // Anything Typed With Multiple Results Gets Printed To The Screen
    rl_variable_bind("show-all-if-ambiguous", "on");
    // Use 'omnish_command_completion' Logic
    rl_attempted_completion_function = omnish_command_completion;

    /** OmniShell Custom Keybinds **/
    // NOTE: TAB '\t'
    omnish_bind_key('\t', omnish_tab_complete);

    // History Search
    const char *up_arrow = rl_get_termcap("ku");
    const char *down_arrow = rl_get_termcap("kd");

    /*
    * Terminals commonly send one of two different
    * escape sequences for the arrow keys.
    *
    * Bind BOTH forms so Readline's default
    * previous-history/next-history bindings cannot win.
    */

    omnish_bind_keyseq("\033[A", rl_history_search_backward);
    omnish_bind_keyseq("\033OA", rl_history_search_backward);

    omnish_bind_keyseq("\033[B", rl_history_search_forward);
    omnish_bind_keyseq("\033OB", rl_history_search_forward);

    /*
    * Also bind whatever the current terminal description
    * explicitly reports.
    */
    if (up_arrow != NULL) {
        omnish_bind_keyseq(up_arrow, rl_history_search_backward);
    }
    if (down_arrow != NULL) {
        omnish_bind_keyseq(down_arrow, rl_history_search_forward);
    }
}

static int omnish_bind_key(int key, rl_command_func_t *function)
{
    // NOTE: int rl_bind_key (int key, rl_command_func_t *function);
    return rl_bind_key(key, function); // some_readline_actions(count, key)
}

static int omnish_bind_keyseq(const char *keyseq, rl_command_func_t *function)
{
    return rl_bind_keyseq(keyseq, function);
}
static int omnish_tab_complete(int count, int key)
{
    if (rl_end == 0) {
        if (rl_last_func != omnish_tab_complete) {
            rl_ding();
            return 0;
        }
        return rl_possible_completions(count, key);
    }
    return rl_complete(count, key);
}

static char **omnish_command_completion(const char *text, int start, int end)
{
    (void)end;

    rl_attempted_completion_over = 0;
    if (start != 0) {
        return NULL;
    }
    if (strchr(text, '/') != NULL) {
        return NULL;
    }
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, omnish_command_generator);
}

static char *omnish_command_generator(const char *text, int state)
{
    static size_t index;
    static size_t text_len;
    static char *path_copy = NULL;
    static char *path_cursor = NULL;
    static const char *path_directory = NULL;
    static DIR *directory = NULL;
    static const char *commands[] = {
        "cd",
        "help",
        "exit",
        "pwd",
        "omnirun",
        "history",
        NULL
    };
    /*
     * Readline sends state == 0 on the first call.
     * Reset everything for a new completion search.
     */
    if (state == 0) {
        index = 0;
        text_len = strlen(text);
        // Clean up anything remaining from the previous completion.
        if (directory != NULL) {
            closedir(directory);
            directory = NULL;
        }
        free(path_copy);
        path_copy = NULL;
        path_cursor = NULL;
        path_directory = NULL;
        // Get PATH without modifying the real environment string.
        const char *env_path = getenv("PATH");
        if (env_path != NULL) {
            size_t path_len = strlen(env_path) + 1;
            path_copy = malloc(path_len);
            if (path_copy != NULL) {
                memcpy(path_copy, env_path, path_len);
                path_cursor = path_copy;
            }
        }
    }
    /*
     * First search OmniShell's builtins.
     */
    while (commands[index] != NULL) {
        const char *command = commands[index++];
        if (strncmp(command, text, text_len) == 0) {
            size_t command_len = strlen(command) + 1;
            char *match = malloc(command_len);
            if (!match) {
                return NULL;
            }
            memcpy(match, command, command_len);
            return match;
        }
    }
    /*
     * Then search executable commands inside every directory in PATH.
     */
    while (path_copy != NULL) {
        /*
         * If there is no currently-open PATH directory,
         * grab and open the next one.
         */
        if (directory == NULL) {
            if (path_cursor == NULL) {
                break;
            }
            char *next_colon = strchr(path_cursor, ':');
            path_directory = path_cursor;
            if (next_colon != NULL) {
                *next_colon = '\0';
                path_cursor = next_colon + 1;
            } else {
                path_cursor = NULL;
            }
            /*
             * An empty PATH entry means the current directory.
             */
            if (*path_directory == '\0') {
                path_directory = ".";
            }
            directory = opendir(path_directory);
            if (directory == NULL) {
                continue;
            }
        }
        /*
         * Continue reading from the current directory.
         *
         * Because 'directory' is static, the next time Readline calls
         * this generator, readdir() continues where it stopped.
         */
        struct dirent *executable_name;
        while ((executable_name = readdir(directory)) != NULL) {
            /*
             * Ignore names that do not begin with what the user typed.
             */
            if (strncmp(executable_name->d_name, text, text_len) != 0) {
                continue;
            }
            /*
             * Build:
             *
             *     /usr/bin + / + cat + \0
             */
            size_t full_path_len = strlen(path_directory) + 1 + strlen(executable_name->d_name) + 1;
            char *executable_full_path = malloc(full_path_len);
            if (!executable_full_path) {
                return NULL;
            }
            size_t dir_len = strlen(path_directory);
            // Catch Any Directories With A '/' At The End Of Path Name
            if (dir_len > 0 && path_directory[dir_len - 1] == '/') {
                snprintf(
                    executable_full_path,
                    full_path_len,
                    "%s%s",
                    path_directory,
                    executable_name->d_name
                );
            } else {
                snprintf(
                    executable_full_path,
                    full_path_len,
                    "%s/%s",
                    path_directory,
                    executable_name->d_name
                );
            }
            /*
             * Make sure this PATH entry is actually executable.
             */
            struct stat file_info;

            if (stat(executable_full_path, &file_info) == 0
                && S_ISREG(file_info.st_mode)
                && access(executable_full_path, X_OK) == 0) {

                size_t executable_len = strlen(executable_name->d_name) + 1;

                char *match = malloc(executable_len);
                if (!match) {
                    free(executable_full_path);
                    return NULL;
                }

                memcpy(match, executable_name->d_name, executable_len);

                free(executable_full_path);

                return match;
            }
            free(executable_full_path);
        }
        /*
         * We exhausted this PATH directory.
         * Close it so the outer loop opens the next PATH directory.
         */
        closedir(directory);
        directory = NULL;
    }
    /*
     * No more matches exist.
     */
    if (directory != NULL) {
        closedir(directory);
        directory = NULL;
    }
    free(path_copy);
    path_copy = NULL;
    path_cursor = NULL;
    path_directory = NULL;
    return NULL;
}
