#include "omnifunc.h"
#include "omnireadline.h"

// Private To This File Only
static char *history_file = NULL;
static int session_start_index;

// Upgrade Readline
void omnish_init_readline(void)
{
    // Call Readline's Default Keybinds
    // Creating Readline To Be Interactive
    rl_initialize();

    // Custom Keybinds

}

// Read From Stdin function
char *omnish_read_line(const char *prompt)
{
    /* Get Line To Parse */
    static char *line = NULL;

    // Get Line Read
    /*  NOTE: --> 'readline' Specifically Waits For User To Input A Command
     *              Even If Input Is An Empty Strings
     */
    line = readline(prompt);
    if (!line) {
        return NULL;
    }

    // Return If 'readline' Was Successful And Not An Empty String
    if (line && *line) {
        add_history(line);
    }
    return line;
}

void omnish_history_init(void)
{
    // Initialize History Library Features
    using_history();

    // Variables For History
    int read_hist_state;
    int write_hist_state;
    char *home;
    size_t history_path_size;

    // Create File Path Name For GNU History
    home = getenv("HOME");
    history_path_size = strlen(home) + strlen("/.omnish_history") + 1;
    history_file = malloc(history_path_size);

    snprintf(history_file, history_path_size, "%s/.omnish_history", home);

    // Read From History
    // NULL Pointer Tells History To Use File '~/.history'
    // This Only Grabs The Last 3000 Commands Ran By User
    // From The Previous Shell Usage
    // Entire History Is Still Not Lost
    read_hist_state = read_history(history_file);
    if (read_hist_state == ENOENT) {
        write_hist_state = write_history(history_file);

        if (write_hist_state != 0) {
            fprintf(stderr, "omnish: history file not be created: %s\n", strerror(write_hist_state));
        }
    }
    if (read_hist_state != 0 && read_hist_state != ENOENT) {
        fprintf(stderr, "omnish: history file not loaded: %s\n", strerror(read_hist_state));
    }
    // 'history_length' Holds The Number Of Lines Loaded
    session_start_index = history_length;
}

void omnish_end_history(void)
{
    int write_hist_state;
    int session_history_length = history_length - session_start_index;
    if (session_history_length > 0) {
        // Ensures That History Of This Session Is Not Overwriting The Last Entry
        write_hist_state = append_history(session_history_length, history_file);
        if (write_hist_state == ENOENT) {
            write_hist_state = write_history(history_file);

            if (write_hist_state != 0) {
                fprintf(stderr, "omnish: history file couldn't be recreated: %s\n", strerror(write_hist_state));
            }
        } else if (write_hist_state != 0) {
            fprintf(stderr, "omnish: current history session couldn't be saved: %s\n", strerror(write_hist_state));
        }
    }
    clear_history();
    free(history_file);
}
