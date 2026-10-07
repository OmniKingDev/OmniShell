#pragma once

// For Every OmniShell Script
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Readline Libraries
#include <readline/readline.h>
#include <readline/history.h>
#include <readline/rltypedefs.h>

// Base Omnishell Colors
// For Program And Argument Colors
#define OSH_BRPINK "\033[1;38;5;213m"
#define OSH_BRPURPLE "\033[1;38;5;135m"

// Foreground Color And More
#define OSH_FG "\033[0;37m"
#define OSH_ERROR "\033[1;91m"
#define OSH_WARNING "\033[1;93m"
#define OSH_SUCCESS "\033[1;92m"
#define OSH_OTHER "\033[1;96m"
#define OSH_RESET "\033[0m"
