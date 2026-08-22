#include "omnibuiltins.h"
#include "omnifunc.h"
#include "omnirun.h"
#include "omnishell.h"


// Global Variable
// Brings Offset Tracking Variable
// First Defined/Called In Main Shell Function In 'omnifunc.c'
extern int session_start_index;

// BOTH Static Types Must Have The Same Ordering
static const char *builtin_command[] = {
    "cd",
    "help",
    "exit",
    "pwd",
    "omnirun",
    "history"
};

/* NOTE: --> Declare An Array Of Pointers To Function Types
 *          Holding The Address Of Each Built-in Function.
 *          Dereference Address To Function Itself.
 *          Every Function Argument(s) Is A 'char **'
 *          Return Type Is An 'int'
 *          So Declare Array Of Function Pointers Then;
 *          Declare The ALREADY Declared Return & Argument(s)
 *          Set In Header File 'omnibuiltins.h'
 * - USAGE:
 *          -------------------------------------------------
 *          '(<function>)(char **)' Is The Same As
 *          'int <function>(char **) {...}'
 *          -------------------------------------------------
 */
static int (*builtin_program[])(char **) = {
    &omnish_cd,
    &omnish_help,
    &omnish_exit,
    &omnish_pwd,
    &omnish_omnirun,
    &omnish_history
};

// Grabs The Number Of Builtin OmniShell Holds
int omnish_num_builtins(void)
{
    // Grab Total Bytes Of Array Then Divide By Number Of Bytes For One Command
    // Gives Total Number Of Items In Array
    return sizeof(builtin_command) / sizeof(builtin_command[0]);
}

// Match Program Name With Builtin
// Otherwise Return -1 If Not Builtin
int omnish_run_builtin(char **program)
{
    for (int i = 0; i < omnish_num_builtins(); i++) {
        if (strcmp(program[0], builtin_command[i]) == 0) {
            // Go To Function Address And Dereference
            // Pass 'program' Into Found Builtin Function
            // Which Sends The Arguments(program) Into The Function
            // Then Returns The Functions Return Type After Execution;
            return (*builtin_program[i])(program);
        }
    }
    return -1;
}

/* Builtin Functions */

// Change Directory
// 'cd' By Itself Takes User To Home Directory
int omnish_cd(char **args)
{
    if (args[1] == NULL) {
        // If 'cd' Takes In No Arguments Then Change Directory To '$HOME' env variable
        if (chdir(getenv("HOME")) != 0) {
            perror("omnish");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("omnish");
        }
    }
    return 1;
}

// Help Functions W/Short Explanations
int omnish_help(char **args)
{
    if (args[1] != NULL) {
        fprintf(stderr, OMNI_ERROR "omnish: Wrong Usage!\nUsage: help\nNO ARGUMENTS NEEDED.\n" OMNI_RESET);
        return 1;
    }

    // Introduction To Shell Usage
    printf(OMNI_BRPINK "OmniKing's OMNISHELL\n" OMNI_RESET);
    printf(OMNI_FG "Enter Program Name AND/OR Arguments To Run Builtin Shell Commands\n" OMNI_RESET);
    printf(OMNI_OTHER "The following are built-in commands:\n" OMNI_RESET);

    // Loop While Printing Every Command In Array Of Commands
    for (int i = 0; i < omnish_num_builtins(); i++) {
        printf(OMNI_BRPURPLE " %s\n" OMNI_RESET, builtin_command[i]);
    }
    printf(OMNI_FG " history - Display commands entered during this session.\n" OMNI_RESET);

    // Further Program Introduction
    printf(OMNI_FG "Use the \'man\' command for more information on programs.\n" OMNI_RESET);
    return 1;
}

// Print Current Working Directory
int omnish_pwd(char **args)
{
    size_t buffsize = OMNI_BUFSIZ;
    char *buff;

    if (args[1] != NULL) {
        fprintf(stderr, OMNI_ERROR "omnish: Wrong Usage!\nUsage: pwd\nNO ARGUMENTS NEEDED.\n" OMNI_RESET);
        return 1;
    }

    buff = malloc(sizeof(char) * buffsize);
    if (!buff) {
        fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed" OMNI_RESET);
        return 1;
    }

    /* AI Written Part - CODEX */
    while (getcwd(buff, buffsize) == NULL) {
        if (errno != ERANGE) {
            perror("omnish");
            free(buff);
            return 1;
        }

        char *new_buff;

        buffsize += OMNI_BUFSIZ;
        new_buff = realloc(buff, sizeof(char) * buffsize);
        if (!new_buff) {
            fprintf(stderr, OMNI_ERROR "omnish: Malloc Failed" OMNI_RESET);
            free(buff);
            return 1;
        }
        buff = new_buff;
    }
    /* AI Written Part - CODEX */
    printf(OMNI_FG "%s\n" OMNI_RESET, buff);
    free(buff);
    return 1;
}

int omnish_history(char **args)
{
    int display_num = 1;

    if (args[1] != NULL) {
        fprintf(stderr, OMNI_ERROR "omnish: Wrong Usage!\nUsage: history\nNO ARGUMENTS NEEDED.\n" OMNI_RESET);
        return 1;
    }

    HIST_ENTRY **history = history_list();

    if (!history) {
        printf(OMNI_OTHER "History currently empty\n" OMNI_RESET);
        return 1;
    }

    for (int i = 0; history[i] != NULL; i++) {
        printf(OMNI_FG "  %d:  %s\n" OMNI_RESET, display_num++, history[i]->line);
    }
    return 1;
}

// Exit Function To Close Shell
int omnish_exit(char **args)
{
    if (args[1] != NULL) {
        fprintf(stderr, OMNI_ERROR "omnish: Wrong Usage!\nUsage: exit\nNO ARGUMENTS NEEDED.\n" OMNI_RESET);
        return 1;
    }
    return 0;
}
