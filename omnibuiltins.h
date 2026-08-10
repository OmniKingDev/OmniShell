#ifndef OMNIBUILTINS_H
#define OMNIBUILTINS_H

#include "omnirun.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int omnish_cd(char **args);
int omnish_help(char **args);
int omnish_exit(char **args);
int omnish_pwd(char **args);
int omnish_history(char **args);

int omnish_num_builtins(void);
int omnish_run_builtin(char **program);
void omnish_store_line(char *line);

#endif
