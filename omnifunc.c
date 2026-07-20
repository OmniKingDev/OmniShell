#define _POSIX_C_SOURCE 200809L

// Main Shell Functions (REPL)
#include "omnifunc.h"

// Shell Builtins Header File
#include "omnibuiltins.h"

// Prototype
static char *omnish_prompt_path(const char *current_dir);

// Single Function Used In omnishell.c To Launch Shell
void omnish(void)
{
    // Set Variables To Take In Arguments
    char *line;
    char **argv;
    char *current_dir;
    char *display_dir;
    int status;

    // Start Loop
    do {
        current_dir = omnish_cwd();
        if (!current_dir) {
            return;
        }

        display_dir = omnish_prompt_path(current_dir);
        if (!display_dir) {
            free(current_dir);
            return;
        }

        // All Made Functions Are In Header File
        // Prompt User
        printf("%s\n😈omnishell ==⇒ ", display_dir);
        free(display_dir);
        free(current_dir);

        // Grab Input From Terminal Console(Standard Input)
        line = omnish_read_line(); // --> UPDATE W/ 'getline' Function

        // Then Parse Line To Separate Commands
        argv = omnish_split_line(line);

        // Grab Status To Confirm Execution Of Arguments
        status = omnish_execute(argv);

        // Free Up Memory Used To Execute Arguments
        free(line);
        free(argv);

    } while (status);
}

/* All Functions OmniShell Utilizes */

// Read From Stdin function
char *omnish_read_line(void)
{
    /* Get Line To Parse */

    // Malloc Memory For Line
    char *line = NULL;

    // Buffer Type Set
    // 'size_t' Unsigned Integer
    size_t buffer = 0;

    // Get Line From Stream Using 'getline'
    // 'ssize_t' Signed Integer In-Case Of Error
    ssize_t command = getline(&line, &buffer, stdin);

    // If 'getline' Failed
    if (command == -1) {
        free(line);
        fprintf(stderr, "omnish: Function \'getline\' failed\n");
        return NULL;
    }

    // Return If 'getline' Was Successful
    return line;
}


// Function Tokenizing Line (Parsing)
char **omnish_split_line(char *line) {
    /* Parse Line Given Into Separate Tokens */

    // 'position' And 'bufsiz' Are Both Integers
    int bufsiz = OMNI_TOK_BUFSIZ, position = 0;

    // Malloc For Each Token
    char **tokens = malloc(sizeof(char *) * bufsiz);
    char *token;
    if (!tokens) {
        fprintf(stderr, "omnish: 01 Malloc Failed To Allocate Memory");
        exit(EXIT_FAILURE);
    }

    // Function 'strtok' To Parse/Tokenize Line
    token = strtok(line, OMNI_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        // Increase Tokens Size With More Pointers To SET Buffer
        if (position >= bufsiz) {
            bufsiz += OMNI_TOK_BUFSIZ;
            // Re-Allocate To Add 64 More Pointers To Strings
            char **new_tokens;

            new_tokens = realloc(tokens, bufsiz * sizeof(char *));
            if (!new_tokens) {
                fprintf(stderr, "omnish: 02 Malloc Failed To Re-Allocate Memory");
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        // 'strtok' Must Be Called Twice In-order To Continue Parsing Same Line
        // Setting Pointer To NULL Tells 'strtok' To Use 'line'
        // Continuing From Where It Left Off
        token = strtok(NULL, OMNI_TOK_DELIM);
    }
    // Terminate Then Return List With NULL At The End
    tokens[position] = NULL;
    return tokens;
}


// Execute Each Token Function
int omnish_launch_program(char **tokens)
{
    // Declare Two Integers Capable Of Holding a PID
    pid_t pid, wpid;
    // Declare Status For 'waitpid' Function
    int status;

    // 'fork' Creates Child Process
    // Child Returns 0 If Successful
    pid = fork();
    if (pid == 0) {
        // 'execvp' Gets Tokens Previously Parsed Then Executes
        // The First Token Must Be Program Name
        // 'v' Stands For Vectors, Token List Of Arguments
        // 'p' Is For PATH (OS finds program path through $PATH)
        if (execvp(tokens[0], tokens) == -1) {
            // Prints Error Given By The Library or Function
            // That Caused Error During Execution
            // Allowing User To Be Guided To Source Problem
            // Giving Main Function Name
            perror("omnish");
        }
        // Exit To Keep Shell Running After Printing Error
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // If Creating Child Process Failes
        // Keep Going After Printing Error
        // Let User Decide IF They Want To Terminate Program
        perror("omnish");
    } else {
        // Process Id Of Child Process
        // 'WUNTRACED' Return If A Child Stopped
        // Parent Waits For Child Process To Stop
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
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
    size_t buffsize = OMNI_BUFSIZ;
    char *buff = malloc(sizeof(char) * buffsize);

    if (!buff) {
        fprintf(stderr, "omnish: 03 Malloc Failed");
        return NULL;
    }

    /* AI Written Part - CODEX */
    while (getcwd(buff, buffsize) == NULL) {
        if (errno != ERANGE) {
            perror("omnish");
            free(buff);
            return NULL;
        }

        char *new_buff;

        buffsize += OMNI_BUFSIZ;
        new_buff = realloc(buff, sizeof(char) * buffsize);
        if (!new_buff) {
            fprintf(stderr, "omnish: 04 Malloc Failed");
            free(buff);
            return NULL;
        }
        buff = new_buff;
    }
    /* AI Written Part - CODEX */

    return buff;
}

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
    while (*position != '\0') {
        // If Character Does NOT Equal '/'
        if (*position != '/' && (position == current_dir || position[-1] == '/')) {
            components++;
        }
        // Check Every Character
        position++;
    }

    // 'components' Represents Folders To Be Seen
    if (components > 5) {
        int remaining = 3;

        // Remove All Folder Names That Aren't The Last 3 Directories
        // If Current Directory Exceeds 5 Directories
        position = current_dir + strlen(current_dir);
        while (position > current_dir) {
            position--;
            if (*position == '/') {
                remaining--;
                if (remaining == 0) {
                    suffix = position + 1;
                    break;
                }
            }
        }

        display_size = strlen(suffix) + 5;
        display_dir = malloc(sizeof(char) * display_size);
        if (!display_dir) {
            fprintf(stderr, "omnish: 03 Malloc Failed");
            return NULL;
        }
        snprintf(display_dir, display_size, ".../%s", suffix);
        return display_dir;
    }

    display_size = strlen(current_dir) + 1;
    display_dir = malloc(sizeof(char) * display_size);
    if (!display_dir) {
        fprintf(stderr, "omnish: 03 Malloc Failed");
        return NULL;
    }
    strcpy(display_dir, current_dir);
    return display_dir;
}
