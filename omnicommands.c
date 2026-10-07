#define _POSIX_C_SOURCE 200809L

#include "omnicommands.h"

// Builtins Osh Provides
static const char *commands[] = {
    "cd",
    "help",
    "exit",
    "pwd",
    "echo",
    "omnirun",
    "history",
    NULL
};

size_t osh_builtin_count(void)
{
    /*
     * Grab Total Bytes Of Array Then Divide By The
     * Number Of Bytes For One Element In The Array.
     */
    return (sizeof(commands)/sizeof(commands[0]));
}

// Return Builtin Name At Given Index
const char *osh_builtin_name(size_t index)
{
    size_t command_count = osh_builtin_count();
    if (index >= command_count - 1 || index < 0) {
        return NULL;
    }
    return commands[index];
}

// Status To Check If Builtin Or Not
int osh_is_builtin(const char *command)
{
    if (!command) return 0;

    for (int i = 0; commands[i] != NULL; i++) {
        if (strcmp(command, commands[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

// Find Out If Command Sits In Users $PATH
char *osh_find_executable(const char *command)
{
    if (!command || *command == '\0') {
        return NULL;
    }

    struct stat command_info;

    /*
     * If User Already Supplied A Path:
     *
     *  ./program
     *  ../program
     *  /usr/bin/program
     *
     */
    if (strchr(command, '/') != NULL) {
        // Grab Status Of File
        if (stat(command, &command_info) != 0) {
            return NULL;
        }
        // Check To See If Its A Regular File
        if (!S_ISREG(command_info.st_mode)) {
            return NULL;
        }
        // Check For Execution Access
        if (access(command, X_OK) != 0) {
            return NULL;
        }

        // Length Of Command Given
        size_t command_len = strlen(command) + 1;
        char *command_path = malloc(command_len);
        if (!command_path) return NULL;
        memcpy(command_path, command, command_len);
        return command_path;
    }

    /*
     * Otherwise Search PATH For The Command.
     */

    // Get Users Directories From $PATH
    const char *env_path = getenv("PATH");
    if (!env_path) { return NULL; }

    // Length Of Full Path
    size_t path_len = strlen(env_path) + 1;
    char *path_copy = malloc(path_len);
    if (!path_copy) return NULL;

    // Create Copy Of Path
    memcpy(path_copy, env_path, path_len);

    // Grab Starting Point
    char *path_cursor = path_copy;

    while (path_cursor != NULL) {
        // Mark The Start Of The Directory
        char *path_directory = path_cursor;
        // Then Start Looking For Delimiter In Full String Path
        char *next_colon = strchr(path_cursor, ':');

        if (next_colon != NULL) {
            // Set Delimiter Found To NUL Byte Character ('\0')
            *next_colon = '\0';
            // Then Set Path Cursor To Look At Next Directory
            path_cursor = next_colon + 1;
        } else {
            // If No More Directories Are Found
            path_cursor = NULL;
        }

        // Empty PATH Entry Means Current Directory
        if (*path_directory == '\0') path_directory = ".";

        // Length Of Directory Found
        size_t directory_len = strlen(path_directory);
        // Length Of Command
        size_t command_len = strlen(command);

        // Full Length Of Directory With Command
        size_t full_path_len = directory_len + command_len + 2;
        char *full_path = malloc(full_path_len);

        if (!full_path) { free(path_copy); return NULL; }

        /*
         * Incase Users Directories In Path Have A
         * '/' Already At The End Of Directory String
         */
        if (directory_len > 0 && path_directory[directory_len - 1] == '/') {
            snprintf(full_path, full_path_len, "%s%s", path_directory, command);
        } else {
            snprintf(full_path, full_path_len, "%s/%s", path_directory, command);
        }

        // Check If Command Is A Valid Executable Program
        if (stat(full_path, &command_info) == 0
            && S_ISREG(command_info.st_mode)
            && access(full_path, X_OK) == 0) {

            // Return Full Path After Freeing Directory Copy
            free(path_copy);
            return full_path;
        }

        // Then Free Full Path
        free(full_path);
    }

    /*
     * Free Path And Return If Command
     * Was Not An Executable Seen In Users $PATH
     */
    free(path_copy);
    return NULL;
}
