#define _POSIX_C_SOURCE 200809L

// Holds All Includes For Shell
#include "omnishell.h"

#include "omnirun.h"

// All Supported Compiler(s) & Interpreter(s)
static const char *commands[] = {
        "python3", "python", "python3.14",
        "gcc", "cc", "clang",
        "g++", "c++", "clang++"
};

/** Static-ONLY Prototype Fucntions **/
static void osh_print_usage(void);
static int osh_is_supplied_command(const char *argument);
static char *osh_supply_command(const char *extension);
static const char *osh_extension(const char *path);
static char *osh_output_path(const char *source, const char *extension);
static char *osh_executable_path(const char *output);
static osh_process_result osh_run_process(char *const program[]);
static void osh_report_result(const char *program,
                                  osh_process_result result);
static void osh_remove_output(const char *output);
static int osh_run_interpreted(const char *source, const char *interpreter);
static int osh_run_compiled(const char *source, const char *compiler,
                                const char *extension);

// Main 'omnirun' Function
int osh_omnirun(char **args)
{
    // 'struct stat' Is A Built-In Complex Struct Declared In '<sys/stat.h>'
    // Designed To Grab Metadata About A File(s) Attributes
    struct stat source_status;
    // For File Extension Characters Like '.c | .py | .cpp'
    const char *extension;

    // If Missing Needed Argument(s)
    if (args[1] == NULL) {
        osh_print_usage();
        return 1;
    }

    // Checks For Not Needed Commands/Flags AND/OR Interpreters
    // 'omnirun' Should Supply That Automatically
    if (args[2] != NULL) {
        if (osh_is_supplied_command(args[1])) {
            fprintf(stderr, OSH_WARNING
                    "osh: omnirun supplies '%s' automatically; pass only the source/main file\n"
                    OSH_RESET,
                    args[1]);
        // Assuming User Added Flags AND/OR 'omnirun' Can't Supply Commands
        } else {
            fprintf(stderr, OSH_WARNING"osh: osh v0.1; omnirun accepts exactly one source file\n"OSH_RESET);
        }
        // Any Other Expectation Is Considered Improper Use
        osh_print_usage();
        return 1;
    }

    // 'stat' Function Gives Us A Struct Of The File
    if (stat(args[1], &source_status) == -1) {
        fprintf(stderr, OSH_ERROR"osh: cannot access '%s': %s\n"OSH_RESET, args[1], strerror(errno));
        return 1;
    }

    // 'sys/stat.h' Provides 'stat' Function
    // The 'stat' Struct '.st_mode' Field Is Used To Provide File Type
    // 'S_ISREG' Checks If File Given Is A Regular File
    if (!S_ISREG(source_status.st_mode)) {
        fprintf(stderr, OSH_ERROR"osh: '%s' is not a regular file\n"OSH_RESET, args[1]);
        return 1;
    }

    // Return String Of File Extension Only
    // Exp: ".c" , ".py" , ".cpp"
    extension = osh_extension(args[1]);
    if (!extension) {
        fprintf(stderr, OSH_ERROR"osh: '%s' has no file extension\n"OSH_RESET, args[1]);
        return 1;
    }


    /* NOTE: HEART OF 'omnirun'!!!
     * --> The Rest Of This Function Is The Reason Why 'omnirun' Works
     * --> The Goal Is To Reduce The Repetitive Process Of Remembering Commands
     * --> Every Future File Type Will Contain It's Own Function If Necessary
     */


    // Find File Type In Supplied Built-In Shell Types
    if (strcmp(extension, ".py") == 0) {
        return osh_run_interpreted(args[1], osh_supply_command(extension));
    }
    if (strcmp(extension, ".c") == 0) {
        return osh_run_compiled(args[1], osh_supply_command(extension), extension);
    }
    if (strcmp(extension, ".cpp") == 0) {
        return osh_run_compiled(args[1], osh_supply_command(extension), extension);
    }

    // If No Supported File Extension Is Found
    fprintf(stderr, OSH_ERROR"osh: unsupported file extension '%s'\n"OSH_RESET, extension);
    return 1;
}

// Function To Print Usage
static void osh_print_usage(void)
{
    fprintf(stderr, OSH_WARNING
            "Usage: omnirun <file>\n"
            "examples:\n"
            "  - omnirun foo.py\n"
            "  - omnirun bar.c\n"
            "  - omnirun baz.cpp\n"
            OSH_RESET);
}

// Function To Store And Check For Supplied Commands
static int osh_is_supplied_command(const char *argument)
{
    // Count Of Array Of Supplied Commands
    size_t command_count = sizeof(commands) / sizeof(commands[0]);

    // Check To See If Command Type Exist
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(argument, commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static char *osh_supply_command(const char *extension)
{
    // Return Supply Command
    if (strcmp(extension, ".py") == 0) {
        return "python3";
    }
    if (strcmp(extension, ".c") == 0) {
        return "gcc";
    }
    if (strcmp(extension, ".cpp") == 0) {
        return "g++";
    }
    return NULL;
}

// Function To Walk And Grab File Extension
static const char *osh_extension(const char *path)
{
    // Check String For The LAST '/' & '.' In The Path Given
    // 'strrchr' Locates A Character Within A String
    const char *slash = strrchr(path, '/');
    const char *dot = strrchr(path, '.');

    // Missing Valid Executable File
    if (!dot || (slash && dot < slash) || dot[1] == '\0') {
        return NULL;
    }
    // Return The String That Begins At '.'
    return dot;
}

static int osh_run_interpreted(const char *source, const char *interpreter)
{
    // Set Array Of Strings For 'execvp' Function
    char *program[] = {
        (char *)interpreter,
        (char *)source,
        NULL
    };
    osh_process_result result = osh_run_process(program);

    osh_report_result(interpreter , result);
    return 1;
}

static int osh_run_compiled(const char *source, const char *compiler,
                                const char *extension)
{
    osh_process_result compile_result;
    osh_process_result run_result;

    // Brings Back Source File Name Without The Extension Attached
    char *output = osh_output_path(source, extension);

    char *executable;
    int output_fd;

    if (!output) {
        fprintf(stderr, OSH_ERROR"osh: could not derive an output path from '%s'\n"OSH_RESET,
                source);
        return 1;
    }

    // Opens New Source Executable; Creates Source If Not Created Already
    // Errors If Source File Descriptor Already Exist
    // 'S_IRWXU' Gives User Permission 'rwx' To Owner Of Newly Created File Descriptor
    output_fd = open(output, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
    if (output_fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, OSH_WARNING
                    "osh: output path '%s' already exists; refusing to overwrite it\n"
                    OSH_RESET,
                    output);
        } else {
            fprintf(stderr, OSH_ERROR"osh: cannot create output '%s': %s\n"OSH_RESET,
                    output, strerror(errno));
        }
        free(output);
        return 1;
    }
    close(output_fd);

    // Declared With Values That 'execvp' Needs To Use Compile Program
    {
        char *compile_program[] = {
            (char *)compiler,
            (char *)source,
            "-o",
            output,
            NULL
        };

        compile_result = osh_run_process(compile_program);
    }

    // If Not Successful Compiling Of Program
    if (compile_result.outcome != OSH_PROCESS_SUCCESS) {
        // Prints Error Results And The Cause
        osh_report_result(compiler, compile_result);
        // Delete The Program From File-System
        // So No Evidence Of Executable Used Exist After Failed Execution
        osh_remove_output(output);
        free(output);
        return 1;
    }

    // Create Executable Path After Compiled Requested Program Exist
    executable = osh_executable_path(output);
    if (!executable) {
        osh_remove_output(output);
        free(output);
        return 1;
    }

    {
        char *run_program[] = {
            executable,
            NULL
        };

        // Execute New Path To Compiled Program
        run_result = osh_run_process(run_program);
    }
    osh_report_result(executable, run_result);

    // Free Everything Once Completed
    /* NOTE:
     *     Removing Below Comment '//' To Line Of Code
     *     Deletes Compiled Executable Program From File-System
     *     After Execution.
     */
    // omnirun_remove_output(output);
    free(executable);
    free(output);
    return 1;
}

static char *osh_output_path(const char *source, const char *extension)
{
    // Number Of Characters Used For Name Of Executable
    size_t output_length = (size_t)(extension - source);
    char *output;

    if (output_length == 0) {
        return NULL;
    }

    output = malloc(output_length + 1);
    if (!output) {
        fprintf(stderr, OSH_ERROR"osh: failed to allocate output path\n"OSH_RESET);
        return NULL;
    }

    memcpy(output, source, output_length);
    output[output_length] = '\0';
    return output;
}

static char *osh_executable_path(const char *output)
{
    size_t path_size;
    char *path;

    // If '/' Already Is Apart Of Pathname
    if (strchr(output, '/')) {
        path_size = strlen(output) + 1;
        path = malloc(path_size);
        if (!path) {
            fprintf(stderr, OSH_ERROR"osh: failed to allocate executable path\n"OSH_RESET);
            return NULL;
        }
        memcpy(path, output, path_size);
        return path;
    }

    path_size = strlen(output) + 3;
    path = malloc(path_size);
    if (!path) {
        fprintf(stderr, OSH_ERROR"osh: failed to allocate executable path\n"OSH_RESET);
        return NULL;
    }
    // Assuming Executable Is In Current Directory Where 'omnirun' Was Used
    snprintf(path, path_size, "./%s", output);
    return path;
}

static void osh_remove_output(const char *output)
{
    // 'unlink' Deletes The File Descriptor That Is No Longer Being Used
    // Removing Executable File From File-System
    if (unlink(output) == -1 && errno != ENOENT) {
        fprintf(stderr, OSH_ERROR"osh: could not remove '%s': %s\n"OSH_RESET,
                output, strerror(errno));
    }
}

// Function That Returns A Return Value Associated With Outcome Enums
static osh_process_result osh_run_process(char *const program[])
{
    // Return Process Result Declared If Failures Occur
    osh_process_result result = {
        .outcome = OSH_PROCESS_SETUP_FAILURE,
        .value = 0
    };
    // Needed For Using 'pipe' & 'fork' Functions
    int pipe_fd[2];
    int status;
    pid_t pid;
    pid_t waited;

    // Create Read & Write End Pipes
    // Returns '-1' If 'pipe' Function Failed
    // Parent Holds 'pipe_fd[0]', The Read File Descriptor
    // Child Holds 'pipe_fd[1]', The Write File Descriptor
    if (pipe(pipe_fd) == -1) {
        perror("osh: pipe");
        return result;
    }

    // 'fcntl' Sets Flags For Read Descriptors
    // This Function Expects Descriptors To Be Open Files
    // 'F_SETFD' Set File Descriptor File Flags
    // 'FD_CLOEXEC' File Descriptor W/ Close-On-Exec Flag  ↓↓↓
    // Exec(any 'exec' family function) Flag Triggers 'FD_CLOEXEC'
    // Returns 0 On Successful
    if (fcntl(pipe_fd[1], F_SETFD, FD_CLOEXEC) == -1) {
        perror("osh: fcntl");
        // Close Files If Failed
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return result;
    }

    // Create Child Process
    pid = fork();
    if (pid == -1) {
        perror("osh: fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        // Set Result To Created Enum Error For 'fork' Function
        result.outcome = OSH_PROCESS_FORK_FAILURE;
        return result;
    }

    // If Child Process Is Successful
    if (pid == 0) {

        // For 'errno' Return Value
        int exec_err;

        // Child Process Should Not Read From Pipe
        // Write End Already Set To Close After Success Of 'execvp'
        close(pipe_fd[0]);

        // Execute Program With 'program' Arguments
        execvp(program[0], program);

        /*
         * BELOW CODE HAPPENS If Unsuccessful Child Process Execution
         * Otherwise Current Process Image Is Changed To Given Program
        */
        exec_err = errno;

        // Write Error Into Child's File Descriptor If Interruption Occured
        // 'EINTR' Sees If Call Was Interrupted By Signal Before Any Data Was Written
        // Loop Ensures All Data Is Written Before Closing Write-End Pipe
        while (write(pipe_fd[1], &exec_err, sizeof(exec_err)) == -1
               && errno == EINTR) {
        }
        close(pipe_fd[1]);
        // '127' Error Status Code Means; "command not found"
        // '_exit' Exits Broken Child Without Cleaning Out Standard Parents I/O Buffers
        _exit(127);
    }

    // Parent Process Should Not Write To Pipe
    // Child Process Has Already Done So
    close(pipe_fd[1]);

    // Wait For Status Of Created Child Process
    // Loop Ensures That Parent Doesn't Stop Waiting Prematurely
    do {
        waited = waitpid(pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    if (waited == -1) {
        perror("osh: waitpid");
        close(pipe_fd[0]);
        result.outcome = OSH_PROCESS_WAIT_FAILURE;
        return result;
    }

    int exec_error;
    ssize_t bytes_read;

    // Read Bytes From Parent File Descriptor
    do {
        bytes_read = read(pipe_fd[0], &exec_error, sizeof(exec_error));
    } while (bytes_read == -1 && errno == EINTR);
    // No Longer Need File Descriptors
    close(pipe_fd[0]);

    // IF 'exec_error' Equals The Error In Bytes Read
    // That Means 'execvp' Failed
    if (bytes_read == (ssize_t)sizeof(exec_error)) {
        // Reset The Value Of 'errno'
        errno = exec_error;
        perror("osh: execvp");
        // Log Result Of Error Read
        result.outcome = OSH_PROCESS_EXEC_FAILURE;
        result.value = exec_error;
        return result;
    }
    // If 'read' Returns An Error
    if (bytes_read == -1) {
        perror("osh: read");
        return result;
    }
    // If Child Was Terminated By A Signal
    if (WIFSIGNALED(status)) {
        result.outcome = OSH_PROCESS_SIGNAL_TERMINATION;
        // 'WTERMSIG' Gives Number Of Signal That Terminated Child
        result.value = WTERMSIG(status);
        return result;
    }
    // If Child Exited Normally By Either Exiting With 'exit' OR '_exit'
    // Or By Returning To Main
    if (WIFEXITED(status)) {
        // 'WEXITSTATUS' Returns Exit Status Of Child
        result.value = WEXITSTATUS(status);
        // 
        result.outcome = result.value == 0
            ? OSH_PROCESS_SUCCESS
            : OSH_PROCESS_EXIT_FAILURE;
        return result;
    }

    return result;
}

static void osh_report_result(const char *program,
                                  osh_process_result result)
{
    switch (result.outcome) {
        case OSH_PROCESS_EXIT_FAILURE:
            fprintf(stderr, OSH_ERROR"osh: %s exited with status %d\n"OSH_RESET,
                    program, result.value);
            break;
        case OSH_PROCESS_SIGNAL_TERMINATION:
            fprintf(stderr, OSH_ERROR"osh: %s terminated by signal %d\n"OSH_RESET,
                    program, result.value);
            break;
        case OSH_PROCESS_SETUP_FAILURE:
            fprintf(stderr, OSH_ERROR"osh: %s failed by 'pipe' function %d\n"OSH_RESET,
                    program, result.value);
            break;
        case OSH_PROCESS_FORK_FAILURE:
            fprintf(stderr, OSH_ERROR"osh: %s failed by 'fork' function %d\n"OSH_RESET,
                    program, result.value);
            break;
        case OSH_PROCESS_WAIT_FAILURE:
            fprintf(stderr, OSH_ERROR"osh: %s failed by 'wait' function %d\n"OSH_RESET,
                    program, result.value);
            break;
        case OSH_PROCESS_EXEC_FAILURE:
            fprintf(stderr, OSH_ERROR"osh: %s failed by 'execvp' function %d\n"OSH_RESET,
                    program, result.value);
            break;
        default:
            break;
    }
}
