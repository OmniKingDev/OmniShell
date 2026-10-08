#include "omnibuiltins.h"
#include "omnicommands.h"
#include "omnifunc.h"
#include "omnirun.h"

/* NOTE: --> Declare An Array Of Pointers To Function Types
 *          Holding The Address Of Each Built-in Function.
 *          Dereference Address To Function Itself.
 *          Every Function Parameter Is A 'char **', Return Type Is An 'int'
 *          So Any Tokenized String Gets Converted Back Into Char **
 *          Which The Builtins Recieve.
 * - USAGE: 'builtin_program[]'
 *          -------------------------------------------------
 *          '(*<function>)(char **line);' Is The Same As
 *          '<function>(char **line);' Normal Func. Use
 *          -------------------------------------------------
 */
static int (*builtin_program[])(char **) = {
    &osh_cd,
    &osh_help,
    &osh_exit,
    &osh_pwd,
    &osh_echo,
    &osh_omnirun,
    &osh_history,
    NULL
};

int osh_run_builtin(char **args)
{
    // Match Program Name With Builtin
    // Otherwise Return -1 If Not Builtin
    size_t program_count = sizeof(builtin_program)/sizeof(builtin_program[0]);
    // Make Sure Both 'omnibuiltins.c' And 'omnicommands.c' Reflect The Same List Of Builtins
    if (!args || !args[0] || !osh_is_builtin(args[0])
        || program_count != osh_builtin_count()) { return -1; }

    for (size_t i = 0; i < program_count; i++) {
        // If Builtin Was Found Then Return Function To Run Passing The Arguments
        if (strcmp(args[0], osh_builtin_name(i)) == 0)
            // Call For That Builtin Function Found
            // NOTE:
            //  Both 'builtin_program' And 'commands' From omnicommands.c File
            //  Must Be Categorically The Same For This To Work
            return (*builtin_program[i])(args);
    }
    // For Anything Else That Fails
    return -1;
}

/* Builtin Functions */

// Change Directory
// 'cd' By Itself Takes User To Home Directory
int osh_cd(char **args)
{
    if (args[1] == NULL) {
        // If 'cd' Takes In No Arguments Then Change Directory To '$HOME' env variable
        if (chdir(getenv("HOME")) != 0) { perror("osh"); }
    } else {
        // Otherwise Take User To Desired Directory IF Plausible
        if (chdir(args[1]) != 0) { perror("osh"); }
    }
    return 1;
}

// TODO: -> Finish upgrading 'echo' command
int osh_echo(char **args)
{
    int newline = 0;
    int first_text = 1;
    int i = 1;
    int j = 1;
    char flag = '-';
    int flags = 0;

    // Print Every Argument After 'echo'
    // Activating Flag(s) If Found
    while(args[i]) {
        // Flags Must Start With '-' Character
        if (args[i][0] == flag) {
            // If Flags Were Not Used Properly
            if (i != 1) {
                fprintf(stderr, OSH_ERROR"osh: Incorrect usage of flag(s)\nFlags must be used as the first argument"OSH_RESET"\nThe first character should begin with '-'\nUsage:\n\techo -<flags> <arguments>\n");
                return 1;
            // Check For Every Flag Given
            } else if (i == 1) {
                // Loop Until The '\0' Byte Is Found
                while (args[i][j] != '\0') {
                    // Turn On Any Valid Flag Found
                    switch (args[i][j]) {
                        case 'n':
                            newline = 1;
                            break;
                        /* Adding '-e' Flag To Interpret Escapes Like '\'
                        *  case 'e':
                        *      interpret_escapes = 1;
                        *      break;
                        */
                        // If Flag Given Was Invalid
                        default:
                            fprintf(stderr, OSH_ERROR"osh: Invalid flag '-%c'\n"OSH_RESET, args[i][j]);
                            return 1;
                    }
                    j++;
                }
                flags++;
            }
        }
        i++;
    }

    // Reset i Back To 1 After Checking Flag
    i = 1 + flags;
    while (args[i]) {

        if (!first_text) {
            printf(" ");
        }

        // Turn Off If We Already Started Printing
        first_text = 0;
        printf("%s", args[i]);
        i++;
    }
    // Print Newline After String(s) Or Newline If There Are No Arguments
    if (!newline) {
        printf("\n");
    }

    return 1;
}

// Help Functions W/Short Explanations
int osh_help(char **args)
{
    if (args[1] != NULL) {
        fprintf(stderr, OSH_ERROR"osh: Wrong Usage!\nUsage: help\nNO ARGUMENTS NEEDED.\n"OSH_RESET);
        return 1;
    }

    // Introduction To Shell Usage
    printf(OSH_BRPINK"Osh - Omnishell Builtins\n"OSH_RESET);
    printf(OSH_FG"Enter program name AND/OR arguments to run builtin shell commands\nIncluding programs found in $PATH\n"OSH_RESET);
    printf(OSH_OTHER"The following are built-in commands:\n\n"OSH_RESET);

    // Loop While Printing Every Command In Array Of Commands
    for (size_t i = 0; i < osh_builtin_count(); i++) {
        printf(OSH_BRPURPLE" %s\n"OSH_RESET, osh_builtin_name(i));
    }

    // Further Program Introduction
    printf(OSH_FG" history - Display all commands entered since \'.osh_history\' was created.\n"OSH_RESET);
    printf(OSH_FG"Use the \'man\' command for more information on programs.\n"OSH_RESET);
    return 1;
}

// Print Current Working DirectoryA
// Plan For pwd To Take In At Least One File Argument
// To Display Full Path Of Given File If Found
int osh_pwd(char **args)
{
    size_t buffsize = DIR_BUFSIZ;
    char *buff;

    if (args[1] != NULL) {
        fprintf(stderr, OSH_ERROR"osh: Wrong Usage!\nUsage: pwd\nNO ARGUMENTS NEEDED.\n"OSH_RESET);
        return 1;
    }

    buff = malloc(sizeof(char) * buffsize);
    if (!buff) {
        fprintf(stderr, OSH_ERROR"osh: Malloc Failed"OSH_RESET);
        return 1;
    }

    /* AI Written Part - CODEX */
    while (getcwd(buff, buffsize) == NULL) {
        if (errno != ERANGE) {
            perror("osh");
            free(buff);
            return 1;
        }

        char *new_buff;

        buffsize += DIR_BUFSIZ;
        new_buff = realloc(buff, sizeof(char) * buffsize);
        if (!new_buff) {
            fprintf(stderr, OSH_ERROR"osh: Malloc Failed"OSH_RESET);
            free(buff);
            return 1;
        }
        buff = new_buff;
    }
    /* AI Written Part - CODEX */
    printf(OSH_FG"%s\n"OSH_RESET, buff);
    free(buff);
    return 1;
}

int osh_history(char **args)
{
    int display_num = 1;

    if (args[1] != NULL) {
        fprintf(stderr, OSH_ERROR"osh: Wrong Usage!\nUsage: history\nNO ARGUMENTS NEEDED.\n"OSH_RESET);
        return 1;
    }

    HIST_ENTRY **history = history_list();

    if (!history) {
        printf(OSH_OTHER"History currently empty\n"OSH_RESET);
        return 1;
    }

    for (int i = 0; history[i] != NULL; i++) {
        printf(OSH_FG"  %d:  %s\n"OSH_RESET, display_num++, history[i]->line);
    }
    return 1;
}

// Exit Function To Close Shell
int osh_exit(char **args)
{
    if (args[1] != NULL) {
        fprintf(stderr, OSH_ERROR "osh: Wrong Usage!\nUsage: exit\nNO ARGUMENTS NEEDED.\n" OSH_RESET);
        return 1;
    }
    return 0;
}
