#include "omnifunc.h"
#include "omnibuiltins.h"

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

