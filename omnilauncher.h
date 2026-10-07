#pragma once

#ifndef omnishell_h
    #include "omnishell.h"
#endif

#include "omniparser.h"

typedef struct {
    char **argv;
    const char *input_path;
    const char *output_path;
    int output_flags;
} OSHCommand;

int osh_execute(OSHToken *tokens);
int osh_launch_program(OSHToken *tokens);
