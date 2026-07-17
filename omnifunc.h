#ifndef OMNIFUNC_H
#define OMNIFUNC_H

// Defining Predetermined Buff/Token Sizes
#define OMNI_BUFSIZ 1024
#define OMNI_TOK_BUFSIZ 64

// Defining Delimeters
#define OMNI_TOK_DELIM " \t\r\n\a"

// Shell Builtins Header File
#include "omnibuiltins.h"

// Read From Stdin function
char *omnish_read_line(void)
{
    /* Get Line To Parse */

    // Starting Buff Size For Stream
    int bufsize = OMNI_BUFSIZ;

    // Starting Position For Reading Stream
    int position = 0;

    // Malloc Memory For Buffer
    char *buffer = malloc(sizeof(char) * bufsize);

    // Declare Variable For Character In Stream
    int c;

    // Check For Invalid Malloc
    if (!buffer) {
        fprintf(stderr, "omnish: 01 Malloc Failed To Allocate Memory\n");
        exit(EXIT_FAILURE);
    }

    // Loop Through Stream
    while (true) {

        // 'getchar' Grabs From Stream places letter in 'c'
        c = getchar();

        // Check Every Character Until Either EOF or '\n' char.
        if (c == EOF || c == '\n') {
            // IF so then place terminating NUL char. then return buffer
            buffer[position] = '\0';
            return buffer;
        // Place Valid Characters Into Buffer
        } else {
            buffer[position] = c;
        }
        // Add To Change Index To Next Character
        // Loop Until 'return buffer;' triggers
        position++;

    }

    // If Stream Exceeds Buffer Size
    if (position >= bufsize) {
        // Double Buffer Size With 'OMNI_BUFSIZ' Then Reallocate Memory
        // Into 'buffer' Until 'bufsize' Is Greater Than 'position'
        bufsize += OMNI_BUFSIZ;
        buffer = realloc(buffer, bufsize);

        // Check For Invalid Malloc
        if (!buffer) {
            fprintf(stderr, "omnish: 02 Malloc Failed To Re-Allocate Memory");
            exit(EXIT_FAILURE);
        }
    }
}


// Function Tokenizing Line (Parsing)
char **omnish_split_line(char *line) {
    /* Parse Line Given Into Separate Tokens */

    // 'position' And 'bufsiz' Are Both Integers
    int bufsiz = OMNI_TOK_BUFSIZ, position = 0;

    // Malloc For Each Token
    char **tokens = malloc(sizeof(char) * bufsiz);
    char *token;
    if (!tokens) {
        fprintf(stderr, "omnish: 03 Malloc Failed To Allocate Memory");
        exit(EXIT_FAILURE);
    }

    // Function 'strtok' To Parse/Tokenize Line
    while ((token = strtok(line, OMNI_TOK_DELIM)) != NULL) {
        tokens[position] = token;
        position++;

        // Increase Tokens Size With More Pointers To SET Buffer
        if (position >= bufsiz) {
            bufsiz += OMNI_TOK_BUFSIZ;
            // Re-Allocate To Add 64 More Pointers To Strings
            tokens = realloc(tokens, bufsiz * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "omnish: 04 Malloc Failed To Re-Allocate Memory");
                exit(EXIT_FAILURE);
            }
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
#endif
