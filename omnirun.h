#pragma once

#ifndef OMNISHELL_H
    #include "omnishell.h"
#endif

// Set Enums For Execution Outcomes
// MUST BE IN THIS ORDER
// Used To Mark Error Codes Into Readable Names
// 'OMNIRUN_PROCESS_SUCCESS' At Index 0
// ↓ ↓ ↓
// Other Enums
// ↓ ↓ ↓
// 'OMNIRUN_PROCESS_SETUP_FAILURE' At Index 6
typedef enum OshrunProcessType {
    OSH_PROCESS_SUCCESS,
    OSH_PROCESS_EXIT_FAILURE,
    OSH_PROCESS_FORK_FAILURE,
    OSH_PROCESS_EXEC_FAILURE,
    OSH_PROCESS_WAIT_FAILURE,
    OSH_PROCESS_SIGNAL_TERMINATION,
    OSH_PROCESS_SETUP_FAILURE
} osh_process_t;

// Struct To Associate Outcome Enum W/
// Return Value/Exit Code Given By Program Ran
typedef struct OshProcessResult {
    osh_process_t outcome;
    int value;
} osh_process_result;

int osh_omnirun(char **args);
