#pragma once

#ifndef OMNISHELL_H
    #include "omnishell.h"
#endif

size_t osh_builtin_count(void);
const char *osh_builtin_name(size_t index);
int osh_is_builtin(const char *command);
// Returned Executable Path Is Dynamically Allocated And Must Be Freed By Caller
char *osh_find_executable(const char *command);
