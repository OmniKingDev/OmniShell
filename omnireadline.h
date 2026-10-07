#pragma once

#ifndef OMNISHELL_H
    #include "omnishell.h"
#endif

void osh_init_readline(void);
void osh_init_keybinds(void);

char *osh_readline(const char *prompt);

void osh_init_history(void);
void osh_end_history(void);
