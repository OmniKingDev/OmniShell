#ifndef OMNIRUN_H
#define OMNIRUN_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Set Enums For Execution Outcomes
// MUST BE IN THIS ORDER
// Used To Mark Error Codes Into Readable Names
// 'OMNIRUN_PROCESS_SUCCESS' At Index 0
// ↓ ↓ ↓
// Other Enums
// ↓ ↓ ↓
// 'OMNIRUN_PROCESS_SETUP_FAILURE' At Index 6
typedef enum Omnirun_Process_Type {
    OMNIRUN_PROCESS_SUCCESS,
    OMNIRUN_PROCESS_EXIT_FAILURE,
    OMNIRUN_PROCESS_FORK_FAILURE,
    OMNIRUN_PROCESS_EXEC_FAILURE,
    OMNIRUN_PROCESS_WAIT_FAILURE,
    OMNIRUN_PROCESS_SIGNAL_TERMINATION,
    OMNIRUN_PROCESS_SETUP_FAILURE
} omnirun_process_t;

// Struct To Associate Outcome Enum W/
// Return Value/Exit Code Given By Program Ran
typedef struct OmnirunProcessResult {
    omnirun_process_t outcome;
    int value;
} omnirun_process_result;

int omnish_omnirun(char **args);

#endif
