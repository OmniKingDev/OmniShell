#ifndef OMNISHELL_H
#define OMNISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Made Functions Used Within omnish
#include "omnifunc.h"

#endif
// Functions Used In omnishell.c
void omnish(void)
{
    // Set Variables To Take In Arguments
    char *line;
    char **argv;
    int status;

    // Start Loop
    do {
        // All Made Functions Are In Header File
        // Prompt User
        printf("😈omnishell⇒ ");

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

