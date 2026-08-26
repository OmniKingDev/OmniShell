#ifndef OMNIREADLINE_H
#define OMNIREADLINE_H

// Readline Libraries
#include <readline/readline.h>
#include <readline/history.h>
#include <readline/rltypedefs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

void omnish_init_readline(void);
void omnish_init_keybinds(void);

char *omnish_read_line(const char *prompt);

void omnish_history_init(void);
void omnish_end_history(void);

#endif
