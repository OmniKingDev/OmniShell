#include "omnibuiltins.h"
#include "omnifunc.h"

// BOTH Static Types Must Have The Same Ordering
static const char *builtin_command[] = {
    "cd",
    "help",
    "exit",
    "pwd"
};

static int (*builtin_program[])(char **) = {
    &omnish_cd,
    &omnish_help,
    &omnish_exit,
    &omnish_pwd
};

// Grabs The Number Of Builtin OmniShell Holds
int omnish_num_builtins(void)
{
    return sizeof(builtin_command) / sizeof(builtin_command[0]);
}

// Match Program Name With Builtin
// Otherwise Return -1 If Not Builtin
int omnish_run_builtin(char **program)
{
    for (int i = 0; i < omnish_num_builtins(); i++) {
        if (strcmp(program[0], builtin_command[i]) == 0) {
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
    // Introduction To Shell Usage
    printf("OmniKing's OMNISHELL\n");
    printf("Enter Program Name AND/OR Arguments To Run Builtin Shell Commands\n");
    printf("The following are built-in commands:\n");

    // Loop While Printing Every Command In Array Of Commands
    for (int i = 0; i < omnish_num_builtins(); i++) {
        printf(" %s\n", builtin_command[i]);
    }

    // Further Program Introduction
    printf("Use the \'man\' command for more information on programs.\n");
    return 1;
}

// Print Current Working Directory
int omnish_pwd(char **args)
{
    size_t buffsize = OMNI_BUFSIZ;
    char *buff = malloc(sizeof(char) * buffsize);

    if (!buff) {
        fprintf(stderr, "omnish: 03 Malloc Failed");
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
            fprintf(stderr, "omnish: 04 Malloc Failed");
            free(buff);
            return 1;
        }
        buff = new_buff;
    }
    /* AI Written Part - CODEX */
    printf("%s\n", buff);
    free(buff);
    return 1;
}

// Exit Function To Close Shell
int omnish_exit(char **args)
{
    return 0;
}
