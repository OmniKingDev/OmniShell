// Holds All Language/POSIX Headers For Shell
#include "omnishell.h"

// All Other 'osh*' Made Functions
// More Info On Functions Are In 'osh*.c' Files
#include "omnireadline.h"
#include "omnifunc.h"

// Used For Finding '.osh_history' File In Users HOME path
static char *history_file = NULL;

// Starting Index For Storing History List
static int session_start_index;

/* Prototypes That 'osh_readline()' Uses */
static void osh_add_history(const char *line);

// Initialize Readling For Interactive Line Editing
void osh_init_readline(void)
{
    /*  NOTE:
     *  Note That Readline's 'readline()' Function Calls This Function
     *  Before Reading A Line. So It's Not Necessary To Call But Pedantic.
     *  Initialize GNU Readline's internal state and load its normal
     *  startup configuration, then apply OmniShell-specific keybinds
     *  and completion behavior on top of it.
     */
    rl_initialize();

    // Load OmniShell Custom Keybinds Extensions
    osh_init_keybinds();
}

// Read From Stdin function
char *osh_readline(const char *prompt)
{
    /* Used For Storing Line 'readline()' Returns */
    static char *line = NULL;

    // Get Line Read
    /* NOTE:
     *  'readline' Specifically Waits For User To Enter An Entry
     *  Even If Entry Is An Empty String Readline Will Accept It
     */
    line = readline(prompt);

    // If 'readline()' Failed Then Return Null
    if (!line) { return NULL; }

    // Return If 'readline()' Was Successful And Entry Is Not An Empty String
    // Store The Valid Entry Into Persistent History File
    if (line && *line) { osh_add_history(line); }

    // Return Line For Later Tokenizing Then Executing String
    return line;
}


// Function To Utilize Tools From GNU History
// For Storing Entry Into Persistent File Without
// Back To Back Duplicate Commands Ran Right Next
// To Eachother Being Stored.
static void osh_add_history(const char *line)
{
    // GNU History Main Struct 'HIST_ENTRY' Used To Grab Or Add Entries Into '.osh_history' File
    // 'history_list()' Used To Return A NULL Terminated Array Of Current History List
    HIST_ENTRY **history = history_list();

    // If History Exist And Has At Least One Entry
    // GNU History Declares A Variable 'history_length'
    // That Stores The Entire History List Length
    if (history != NULL && history_length > 0) {

        // Look At The Last Entry Entered Currently
        HIST_ENTRY *last_entry = history[history_length - 1];

        // If Current Entry Line And Previous Line Match
        // Then Ignore And Don't Store Into File
        if (strcmp(last_entry->line, line) == 0) { return; }
    }

    // Store Valid Entries Into File '.osh_history'
    add_history(line);
}

// Initialize GNU History With Attached Custom Shell File
void osh_init_history(void)
{
    // Initialize History Library Features Functions/Interactive Variables
    using_history();

    // For GNU History Function 'read_history()' Returns 0 On Success
    int read_hist_state;

    // For GNU History Function 'write_history()' Returns 0 On Success
    int write_hist_state;

    // Used For Users Global $HOME Environment Path
    char *home;

    // Size Of History File Path
    size_t history_path_size;

    // Grab Users File Path Name Of Thier Home Folder
    home = getenv("HOME");

    // Find Length Of User's Home Path Attached With Custom Shell History File Name
    history_path_size = strlen(home) + strlen("/.osh_history") + 1;

    // Malloc Heap Memory
    history_file = malloc(history_path_size);

    // If Malloc Failed
    if (!history_file) {
        fprintf(stderr, OSH_ERROR"osh: Function 'osh_init_history':\nMalloc failed to allocate memory for '~/.osh_history'\n"OSH_RESET);
        return;
    }

    // Print Full Shell History File Path Name Into Buffer
    snprintf(history_file, history_path_size, "%s/.osh_history", home);

    // Read From Given Shell History File
    read_hist_state = read_history(history_file);

    // If File '~/.osh_history' Does NOT Exist
    if (read_hist_state == ENOENT) {

        // If File Does Not Already Exist Then File Is Created Instead
        write_hist_state = write_history(history_file);

        // If File Could Not Be Created
        if (write_hist_state != 0) { fprintf(stderr, OSH_ERROR"osh: Function 'osh_init_history()':\nHistory file '~/.osh_history' cannot be created: %s\n"OSH_RESET, strerror(write_hist_state)); }
    }

    // If 'read_history()' Failed For Any Other Reason
    if (read_hist_state != 0 && read_hist_state != ENOENT) { fprintf(stderr, OSH_ERROR"osh: Function 'osh_init_history()':\nHistory file '~/.osh_history' not loaded: %s\n", strerror(read_hist_state)); }

    // 'history_length' Holds The Number Of Lines Loaded
    // Start Current Shell Session At The Index Of Last Entry
    session_start_index = history_length;
}

// Write/Append Current Shell Session Into '~/.osh_history'
void osh_end_history(void)
{
    // For Checking If File During Session Was Destroyed
    int write_hist_state;

    // Get Length Of Every Entry Entered During Current Session
    int session_history_length = history_length - session_start_index;

    // If At Least One Entry Was Entered Since The Start Of Session
    if (session_history_length > 0) {

        // Append Only How Every Many Entries Were Entered During Shell Session
        write_hist_state = append_history(session_history_length, history_file);

        // If 'append_history()' Failed Cause File Does Not Exist After Closing Shell Loop
        if (write_hist_state == ENOENT) {

            // Use 'write_history()' To Re-Create File '~/.osh_history'
            write_hist_state = write_history(history_file);

            // If 'write_history()' Failed To Create File
            if (write_hist_state != 0) { fprintf(stderr, OSH_ERROR"osh: Function: 'osh_end_history()':\nHistory file couldn't be recreated: %s\n"OSH_RESET, strerror(write_hist_state)); }

        // If 'append_history()' Failed For Any Other Reason
        } else if (write_hist_state != 0) { fprintf(stderr, OSH_ERROR"osh: Function: 'osh_end_history()':\nCurrent history session couldn't be saved: %s\n"OSH_RESET, strerror(write_hist_state)); }
    }

    // Deletes The Shell's Internal Memory Of History Entries
    clear_history();

    // Free Memory After Shell Clears Internal History
    free(history_file);
}
