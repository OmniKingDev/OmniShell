#pragma once

#ifndef OMNISHELL_H
    #include "omnishell.h"
#endif

int osh_cd(char **args);
int osh_help(char **args);
int osh_exit(char **args);
int osh_pwd(char **args);
int osh_history(char **args);
int osh_echo(char **args);

int osh_num_builtins(void);
int osh_run_builtin(char **args);
