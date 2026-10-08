// Holds All Language/POSIX Headers For Shell
#include "omnishell.h"

// All Other 'osh*' Made Functions
// More Info On Functions Are In 'osh*.c' Files
#include "omnifunc.h"
#include "omnicommands.h"
#include "omnireadline.h"

// Function That Binds Custom Function With Given Key
static int osh_bind_key(int key, rl_command_func_t *function);

// Function That Binds Custom Function With Given Keyseq
static int osh_bind_keyseq(const char *keyseq, rl_command_func_t *function);

// Tab Complete Function
static int osh_tab_complete(int count, int key);
static char **osh_command_completion(const char *text, int start, int end);
static char *osh_command_generator(const char *text, int state);

/* All 'rl*' Variables/Functions Are Used By GNU Readline */

// Initiate Custom Shell Keybinds
void osh_init_keybinds(void)
{

    // Is Used For Displaying A Number Of Matches Of 100 Or More
    // Readline Will Prompt User If Matches Exceed Given Value (100)
    // Otherwise It Will Just Display Results 100 Or Less
    rl_completion_query_items = 100;

    // Keep Completion Results Alphabetical
    rl_sort_completion_matches = 1;

    // Print Duplicate Commands Found In Path Only Once If found
    rl_ignore_completion_duplicates = 1;

    // Anything Not Typed Is Ambiguous So All Results Gets Printed To The Screen
    rl_variable_bind("show-all-if-ambiguous", "on");

    // Changes Readline To Point To An Alternative Completion Function 'osh_command_completion()'
    // 'rl_complete()' With Call This New Given Function Name
    rl_attempted_completion_function = osh_command_completion;

    //    OmniShell Custom Keybinds
    /**  NOTE:
    *    'osh_bind_key()' Takes A Key
    *    Then Attaches That To A Function Like
    *    'osh_tab_complete()' For Example.
    *    More Info On Each Function Shown Is Down Below
    **/

    osh_bind_key('\t', osh_tab_complete); // TAB '\t' Key

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

    osh_bind_keyseq("\033[A", rl_history_search_backward);
    osh_bind_keyseq("\033OA", rl_history_search_backward);

    osh_bind_keyseq("\033[B", rl_history_search_forward);
    osh_bind_keyseq("\033OB", rl_history_search_forward);

    /*
    * Also bind whatever the current terminal description
    * explicitly reports.
    */
    if (up_arrow != NULL) {
        osh_bind_keyseq(up_arrow, rl_history_search_backward);
    }
    if (down_arrow != NULL) {
        osh_bind_keyseq(down_arrow, rl_history_search_forward);
    }
}

// Key Bind Given Function And Key
static int osh_bind_key(int key, rl_command_func_t *function)
{
    // Used To Check For Valid Key
    int valid_key;

    // Bind Given Function And Key Together On Keymap
    valid_key = rl_bind_key(key, function);

    // Check To See If Key Was Invalid
    if (valid_key != 0) { fprintf(stderr, OSH_ERROR"osh: Function 'osh_bind_key()':\nInvalid Key %c\n"OSH_RESET, key); return 1; }

    // Return Result If Valid
    return valid_key;
}

// Binding Given Keyseq And Function
static int osh_bind_keyseq(const char *keyseq, rl_command_func_t *function)
{
    // Used To Check For Valid Keyseq
    int valid_keyseq;

    // Bind Given Function And Keyseq Together On Keymap
    valid_keyseq = rl_bind_keyseq(keyseq, function);

    // Check To See If Keyseq Was Invalid
    if (valid_keyseq != 0) { fprintf(stderr, OSH_ERROR"osh: Function 'osh_bind_keyseq()':\nInvalid Keyseq '%s'\n"OSH_RESET, keyseq); return 1; }

    // Return Result If Valid
    return valid_keyseq;
}

// Create Functionality Of The Tab Key '\t'
static int osh_tab_complete(int count, int key)
{
    // 'rl_end' Looks At The End Of Current Line Buffer
    // If Current Line Buffer Is Empty And Tab Was Entered
    if (rl_end == 0) {

        // If The Address Of The Last Function Readline Ran Is Not 'omnish_tab_complete()'
        // 'rl_ding' Rings The Terminals Bell To Alert User
        if (rl_last_func != osh_tab_complete) { rl_ding(); return 0; }

        // Return All Ambiguous System Commands Possible In Alphabetical Order
        return rl_possible_completions(count, key);
    }

    // Return Possible Completions For Any Non-Empty Line Buffer
    /*
    * 'rl_complete()' Utilizes Our Shell Made Function 'osh_command_completion()'
    *  Set In Variable 'rl_attempted_completion_function' Above
    */
    return rl_complete(count, key);
}

// Made Function Given To Readline To Use When Looking For Possible Commands
static char **osh_command_completion(const char *text, int start, int end)
{
    // '(void)end' Being Casted To No-Op Statement
    // This Allows The Compiler To Ignore The Non Use
    // Of 'end' Variable Given By 'rl_complete()'
    (void)end;

    // Turn On Default Filename Completion
    rl_attempted_completion_over = 0;

    // If Starting Line Buffer Index Is Not The First Word Of Given Line Buffer
    if (start != 0) { return NULL; }

    // If The First Word Is A Directory Path
    if (strchr(text, '/') != NULL) { return NULL; }

    // Turn Off Default Filename Completion If Neither Of The Above Checks Were True
    rl_attempted_completion_over = 1;

    // Return All Possible Commands Based On Current Line Buffer
    // 'osh_command_generator()' Returns One String At A Time
    return rl_completion_matches(text, osh_command_generator);
}

// Used For Generating Individual Options Both Commands And Built-ins
static char *osh_command_generator(const char *text, int state)
{
    // Index For Searching Shell Built-ins
    static size_t index;

    // Length Of Current Line Buffer
    static size_t text_len;

    // Copy Of Full String Found In PATH
    static char *path_copy = NULL;

    // Path Cursor To Keep Track Of Files In Open Directory
    static char *path_cursor = NULL;

    // Individual Directory String Split From PATH
    static const char *path_directory = NULL;

    // Current Directory Open
    static DIR *directory = NULL;
    /*
     * Readline Sends State == 0 On The First Call
     * And Non-Zero On Subsequent Call.
     * Reset Everything For A New Completion Search.
     */
    if (state == 0) {

        // Reset Index To Be At Zero For New Search
        index = 0;

        // Grab Length Of Current Line Buffer
        text_len = strlen(text);

        // Clean Up Anything Remaining From The Previous Completion.
        // If Directory Was Open From Previous Search Call
        // Close Directory If A Directory Was Found Open
        // Ensure To Reset Directory To NULL
        if (directory != NULL) { closedir(directory); directory = NULL; }

        // Free Current PATH String Path Copy
        free(path_copy);

        // Reset All Variables To Null For New Search
        path_copy = NULL;
        path_cursor = NULL;
        path_directory = NULL;

        // Get Full String Of PATH
        const char *env_path = getenv("PATH");

        // Make Sure 'getenv()' Did Not Return NULL
        if (env_path != NULL) {

            // Length Of Full PATH String
            size_t path_len = strlen(env_path) + 1;

            // Malloc Heap For Full PATH String
            path_copy = malloc(path_len);

            // If Malloc Failed
            if (!path_copy) { fprintf(stderr, OSH_ERROR"osh: Function: 'osh_command_generator':\nMalloc failed to allocate memory\n"OSH_RESET); return NULL; }

            // Copy Full Path Into Variable To Later Split String
            memcpy(path_copy, env_path, path_len);

            // Grab String Pointer From 'path_copy'
            path_cursor = path_copy;
        }
    }
    /*
     * First Search OmniShell's Builtins.
     */
    // For Grabbing Shell Built-in Commands
    const char *command;

    // 'osh_builtin_name()' Returns String Of Command Located At Index
    while ((command = osh_builtin_name(index)) != NULL) {

        // Index To Next Built-in Command After Finding The First Command
        index++;

        // Compare Current Line Buffer Text Of N Number Of Characters To Built-in Command
        if (strncmp(command, text, text_len) == 0) {

            // If Matches Found Then Grab Length Of Command Found
            size_t command_len = strlen(command) + 1;

            // Malloc Heap Of Command
            char *match = malloc(command_len);

            // If Malloc Failed
            if (!match) { fprintf(stderr, OSH_ERROR"osh: Function: 'osh_command_generator':\nMalloc failed to allocate memory\n"OSH_RESET); return NULL; }

            // Copy Found Command Into New Heap Memory
            memcpy(match, command, command_len);

            // Return Each Match Found
            return match;
        }
    }
    /*
     * Then Search Executable Commands Inside Every Directory In PATH.
     */
    // Make Sure PATH String Is Not NULL
    while (path_copy != NULL) {
        /*
         * If There Is No Currently Open PATH Directory,
         * Grab And Open The Next One.
         */

        // First Call To Function Will Have Directory Set To NULL
        if (directory == NULL) {

            // If Path Cursor Is Still Set To NULL Up To This Point
            // Means There Is No Directory Path Available To Use
            // Break Out Of Loop TO Read From "Open Directory"
            if (path_cursor == NULL) { break; }

            // Return A Pointer To Every Encounter Of Character Delimiter ':'
            char *next_colon = strchr(path_cursor, ':');

            // Grab Pointer To Start Of Directory String Right Before Delimiter ':'
            path_directory = path_cursor;

            // If Not NULL Then Delimiter Was Found
            if (next_colon != NULL) {
                *next_colon = '\0';
                path_cursor = next_colon + 1;
            } else { path_cursor = NULL; }
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
            if (!executable_full_path) { return NULL; }

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
