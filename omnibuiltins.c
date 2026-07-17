#include "omnibuiltins.h"

static const char *builtin_command[] = {
    "cd",
    "help",
    "exit"
};

static int (*builtin_program[])(char **) = {
    &omnish_cd,
    &omnish_help,
    &omnish_exit
};
