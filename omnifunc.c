#define _POSIX_C_SOURCE 200809L

// Main Shell Functions (REPL)
#include "omnifunc.h"
#include "omnishell.h"

// Shell Builtins Header File
#include "omnibuiltins.h"

// For History Starting Index Used In 'omnibuiltins.c'
int session_start_index = 0;
// Prototypes
static char *omnish_prompt_path(const char *current_dir);
static void omnish_print_banner(void);

// Used In omnishell.c To Launch Shell
void omnish(void)
{
    // Set Variables To Take In Arguments
    // For Functions Later Used To Readline
    // Displaying Current Directory User Is In
    char *line;
    char **argv;
    char *current_dir;
    char *display_dir;
    int status;

    // Initialize Keybinds Readline Provides
    // Print Banner
    omnish_init_readline();
    omnish_print_banner();

    // Initialize History Library Features
    using_history();

    // Variables For History
    int read_hist_state;
    int write_hist_state;
    char *home;
    size_t history_path_size;
    char *history_file;

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
    // Start Loop
    do {
        current_dir = omnish_cwd();
        // Just In-case 'omnish_cwd' Returns Nothing
        if (!current_dir) {
            return;
        }

        // Returns Shorter Version Of Current Working Directory Path
        // ONLY If Path Exceeds Set Limit For Directories Shown
        display_dir = omnish_prompt_path(current_dir);
        if (!display_dir) {
            free(current_dir);
            return;
        }

        /* NOTE:
         *  All 'omni*' Made Functions Are Declared In Header File 'omnifunc.h'
         *  EXCEPT For Any Function With A 'static' Type Remains In This File
         */

        // Prompt User
        char *omnish_prompt = "\n\001" OMNI_BRPINK "\002omnishellv0.1\001" OMNI_RESET "\002 ==> ";
        size_t prompt_size = strlen(display_dir) + strlen(omnish_prompt) + 1;

        char *prompt = malloc(prompt_size);

        if (!prompt) {
            free(display_dir);
            free(current_dir);
            return;
        }

        // Final Prompt User Sees
        snprintf(prompt, prompt_size, "%s%s", display_dir, omnish_prompt);

        // Free Memory After Use
        free(display_dir);
        free(current_dir);

        // Then Hand Prompt To Use In 'readline' Function
        line = omnish_read_line(prompt);
        free(prompt);
        // If 'omnish_read_line' Returns NULL
        // Close Shell Loop
        if (!line) {
            break;
        }

        // Then Parse Line To Separate Commands
        argv = omnish_split_line(line);

        // Grab Status To Confirm Execution Of Arguments
        status = omnish_execute(argv);

        // Free Up Memory Used To Execute Arguments
        free(line);
        free(argv);

    } while (status);

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

/* NOTE: All Functions OmniShell Utilizes */

static void omnish_print_banner(void)
{
    printf(
OMNI_BRPINK
" ▒█████   ███▄ ▄███▓ ███▄    █  ██▓  ██████  ██░ ██ ▓█████  ██▓     ██▓    \n"
"▒██▒  ██▒▓██▒▀█▀ ██▒ ██ ▀█   █ ▓██▒▒██    ▒ ▓██░ ██▒▓█   ▀ ▓██▒    ▓██▒    \n"
"▒██░  ██▒▓██    ▓██░▓██  ▀█ ██▒▒██▒░ ▓██▄   ▒██▀▀██░▒███   ▒██░    ▒██░    \n"
"▒██   ██░▒██    ▒██ ▓██▒  ▐▌██▒░██░  ▒   ██▒░▓█ ░██ ▒▓█  ▄ ▒██░    ▒██░    \n"
"░ ████▓▒░▒██▒   ░██▒▒██░   ▓██░░██░▒██████▒▒░▓█▒░██▓░▒████▒░██████▒░██████▒\n"
"░ ▒░▒░▒░ ░ ▒░   ░  ░░ ▒░   ▒ ▒ ░▓  ▒ ▒▓▒ ▒ ░ ▒ ░░▒░▒░░ ▒░ ░░ ▒░▓  ░░ ▒░▓  ░\n"
"  ░ ▒ ▒░ ░  ░      ░░ ░░   ░ ▒░ ▒ ░░ ░▒  ░ ░ ▒ ░▒░ ░ ░ ░  ░░ ░ ▒  ░░ ░ ▒  ░\n"
"░ ░ ░ ▒  ░      ░      ░   ░ ░  ▒ ░░  ░  ░   ░  ░░ ░   ░     ░ ░     ░ ░   \n"
"    ░ ░         ░            ░  ░        ░   ░  ░  ░   ░  ░    ░  ░    ░  ░\n"
OMNI_BRPURPLE
"                         OmniShell v0.1                                    \n"
"             Welcome To The First Version Of OmniShell!!!                  \n"
OMNI_RESET);
}

// Function Tokenizing Line (Parsing)
char **omnish_split_line(char *line) {
    /* Parse Line Given Into Separate Tokens */

    // 'position' And 'bufsiz' Are Both Integers
    int bufsiz = OMNI_TOK_BUFSIZ;
    int position = 0;

    // Malloc For Each Token
    char **tokens = malloc(sizeof(char *) * bufsiz);
    char *token;
    if (!tokens) {
        fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed To Allocate Memory\n" OMNI_RESET);
        exit(EXIT_FAILURE);
    }

    // Function 'strtok' To Parse/Tokenize Line
    token = strtok(line, OMNI_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        // Increase Tokens Size With More Pointers To SET Buffer
        // ONLY If 'position' Exceeds 'bufsiz'
        if (position >= bufsiz) {
            bufsiz += OMNI_TOK_BUFSIZ;
            char **new_tokens;

            new_tokens = realloc(tokens, bufsiz * sizeof(char *));
            if (!new_tokens) {
                fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed To Re-Allocate Memory\n" OMNI_RESET);
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        // 'strtok' Must Be Called Twice In-order To Continue Parsing Same 'line'
        // Setting Pointer To NULL Tells 'strtok' To Use 'line'; Utilizing Hidden Pointers
        // Setting the Pointer To NULL; NOT 'line' Itself
        // Loops Back To First Call To 'strtok' Grabbing Next Token In 'line'
        token = strtok(NULL, OMNI_TOK_DELIM);
    }
    // Terminate Then Return List With NULL At The End
    // 'execvp' Expects 'tokens' Array To End With NULL
    tokens[position] = NULL;
    return tokens;
}

// Execute Each Token Function
int omnish_launch_program(char **tokens)
{
    // Declare 'pid' Capable Of Holding a PID
    pid_t pid;
    pid_t wpid;
    // Declare Status For 'waitpid' Function
    int status;

    // 'fork' Creates A Child Process Within Parent Process
    // Child Return Value Is 0 As Type 'pid_t' If Successful
    pid = fork();
    if (pid == 0) {
        // 'execvp' Gets Tokens Previously Parsed Then Executes
        // The First Token Must Be Program Name
        // 'v' Stands For Vectors, Token List Of Arguments
        // 'p' Is For PATH (OS finds program path through $PATH)
        if (execvp(tokens[0], tokens) == -1) {
            // Prints Error Given By The Library or Function
            // Allowing User To Be Guided To Source Problem
            // Giving Main Function Name
            perror("omnish");
            // '_exit' The Child's Original Process Image (Copy Of Shell Program(Parent Process))
            // NOT The Shell Program(Parent) The Child Was Created In
            _exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("omnish");
    } else {
        // Results Of Child Process
        // 'WUNTRACED' Return If A Child Stopped
        // Parent Waits For Child Process To Change State
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("waitpid");
                return -1;
            }
        // Loop While Status Doesn't Signify A Properly
        // Exited Or Signaled Terminated Child Process
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    // Return 1 After Success Of Execution
    return 1;
}

// Execute Either Builtins Or System Programs
int omnish_execute(char **program)
{
    // Set Integer For Status
    int builtin_status;

    // If No User Input, Prompt User
    if (program[0] == NULL) {
        return 1;
    }

    // 'omnish_run_builtin' Function In omnibuiltins.h
    builtin_status = omnish_run_builtin(program);

    // Builtin Function Was Found
    if (builtin_status != -1) {
        return builtin_status;
    }

    // Launch System Program If Not Builtin
    return omnish_launch_program(program);
}

// Function For Current Working Directory
char *omnish_cwd(void)
{
    // Allocate Memory For Directory String
    size_t buffsize = OMNI_BUFSIZ;
    char *buff = malloc(sizeof(char) * buffsize);

    if (!buff) {
        fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed\n" OMNI_RESET);
        return NULL;
    }

    /* AI Written Part - CODEX */
    while (getcwd(buff, buffsize) == NULL) {
        // If 'errno' Returns Anything Besides 'ERANGE'
        if (errno != ERANGE) {
            perror("omnish");
            free(buff);
            return NULL;
        }

        char *resized_buff;

        buffsize += OMNI_BUFSIZ;
        resized_buff = realloc(buff, sizeof(char) * buffsize);
        if (!resized_buff) {
            fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed\n" OMNI_RESET);
            free(buff);
            return NULL;
        }
        buff = resized_buff;
    }
    /* AI Written Part - CODEX */

    return buff;
}

/*
* AI Written Function 'omnish_prompt_path'
* Developer Written Comments Of Function
*/
// Shorten Current Working Directory For Prompt
static char *omnish_prompt_path(const char *current_dir)
{
    // Set Variables
    int components = 0;
    const char *position = current_dir;
    const char *suffix = current_dir;
    size_t display_size;
    char *display_dir;

    // Check For The NUL Byte Character
    // When Dereferencing Character Pointers
    while (*position != '\0') {
        // If Character Does NOT Equal '/'
        if (*position != '/' && (position == current_dir || position[-1] == '/')) {
            components++;
        }
        // Only When '/' Is Found In String
        position++;
    }

    // 'components' Represents Folders To Be Seen
    if (components > 5) {
        int remaining = 3;

        // Remove All Folder Names That Aren't The Last 3 Directories
        // If Current Directory Length Exceeds 5 Directories
        position = current_dir + strlen(current_dir);
        while (position > current_dir) {
            position--;
            if (*position == '/') {
                remaining--;
                if (remaining == 0) {
                    // Suffix Should Hold Index At The First Character
                    // Of The Third To Last Directory
                    suffix = position + 1;
                    break;
                }
            }
        }

        // '5' Is For The Extra Characters Printed First
        // Before The Directories Are Displayed
        display_size = strlen(suffix) + 5;
        display_dir = malloc(sizeof(char) * display_size);
        if (!display_dir) {
            fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed\n" OMNI_RESET);
            return NULL;
        }
        snprintf(display_dir, display_size, ".../%s", suffix);
        return display_dir;
    }

    // If File Path Components Is Less Than OR Equal To 5
    display_size = strlen(current_dir) + 1;
    display_dir = malloc(sizeof(char) * display_size);
    if (!display_dir) {
        fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed\n" OMNI_RESET);
        return NULL;
    }
    strcpy(display_dir, current_dir);
    return display_dir;
}
