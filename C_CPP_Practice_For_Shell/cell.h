#ifndef CELL_H
#define CELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/*
 * ANSI Color Codes For Terminal Output Formatting:
 ** Y  - Yellow
 ** G  - Green
 ** C  - Cyan
 ** RED  - Red
 ** RST  - Reset To Default Color
 */
#define Y    "\033[1;33m"
#define G    "\033[1;32m"
#define C    "\033[1;36m"
#define RED  "\033[1;31m"
#define RST  "\033[0m"
#define P    "\033[1;35m"

// Takes In 'printf' Function Then Creates Shortcut
#define p(...) printf(__VA_ARGS__)

// Wrapper For Handling Errors
void Getcwd(char *, size_t);
void printbanner(void);

#endif
