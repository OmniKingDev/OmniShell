#ifndef OMNIFUNC_H
#define OMNIFUNC_H

// Defining Predetermined Buff/Token Sizes
#define OMNI_BUFSIZ 1024
#define OMNI_TOK_BUFSIZ 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

// Defining Delimeters
#define OMNI_TOK_DELIM " \t\r\n\a"

void omnish(void);
char *omnish_read_line(const char *prompt);
char *omnish_cwd(void);
char **omnish_split_line(char *line);
int omnish_launch_program(char **tokens);
int omnish_execute(char **program);

#endif
