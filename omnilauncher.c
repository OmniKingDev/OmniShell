#define _POSIX_C_SOURCE 200809L

#include "omnilauncher.h"
#include "omnibuiltins.h"
#include "omnicommands.h"

/*
 * NOTE:
 *  OSHCommand Struct:
 *      typedef struct {
 *          char **argv;
 *          const char *input_path;
 *          const char *output_path;
 *          int output_flags;
 *      } OSHCommand;
 */

static size_t osh_find_command_end(const OSHToken *tokens, size_t start);
static int osh_validate_commands(const OSHToken *tokens);
static int osh_build_command(const OSHToken *tokens, size_t start, size_t end,
                                OSHCommand *command);
static int osh_apply_redirections(const OSHCommand *command);
static int osh_run_parent_builtin(OSHCommand *command);
static int osh_wait_for_children(const pid_t *children, size_t child_count,
                                    pid_t final_child);

// Execute Tokenized Array
int osh_execute(OSHToken *tokens)
{
    // Return If NO Tokens Exist
    if (!tokens) return 1;

    // Launch Tokens
    int status = osh_launch_program(tokens);
    // Free Heap Memory Of Tokens
    osh_free_tokens(tokens);
    // Return Status Of Tokens Ran
    // If A Pipe Operator '|' Was Used
    // Then Status Returned Is The Last Program To Run
    return status;
}

// Launch Programs Using Tokenized Strings
int osh_launch_program(OSHToken *tokens)
{
    // If Tokens Don't Exist Or Is EOF, Then Return 1
    if (!tokens || tokens[0].token.type == TOKEN_EOF) return 1;
    // Make Sure Tokens Carry Valid Commands
    if (osh_validate_commands(tokens) != 0) return 1;

    // Set To 1 Knowing We have At Least One Command To Process
    size_t command_count = 1;

    // Count How Many Commands That Will Change A Child Process
    for (size_t i = 0; tokens[i].token.type != TOKEN_EOF; i++) {
        // Add 1 To Command Count If Pipe Operator Was Found
        if (tokens[i].token.type == TOKEN_PIPE) command_count++;
    }

    // Find The Ending Index For The First Command
    size_t first_end = osh_find_command_end(tokens, 0);
    // Create The 'OSHCommand' Struct For First Command
    OSHCommand first_command = {0};
    // Since 'exec' Family Function Expects A 'char **' Parameter, Not Struct Types
    // We Need To Convert The Command To A More 'exec()' Friendly Struct Type
    // Also Shell Builtins Expect A 'char **' As Well
    // Return 1 If Something Failed
    if (osh_build_command(tokens, 0, first_end, &first_command) != 0) return 1;


    /*
     * NOTE:
     *      This Is Where We Start Executing Commands. If Builtin
     *      Was Presented After Building The New Struct 'OSHCommand',
     *      Then We Start Doing Builtin First.
     */

    // Check If Command Was Builtin, If So Then Return Status Of Builtins
    if (command_count == 1 && osh_is_builtin(first_command.argv[0])) {
        // Run Builtin Which Means We Can Free The Command/Arguments Right After
        int status = osh_run_parent_builtin(&first_command);
        free(first_command.argv);
        return status;
    }
    // Free The Command/Arguments Used For Builtins Cause They Are No Longer Needed
    free(first_command.argv);

    // Heap Malloc For How Many Children Processes We need
    pid_t *children = malloc(sizeof(pid_t) * command_count);
    // If Malloc Failed
    if (!children) {
        fprintf(stderr, OSH_ERROR"osh: malloc failed to allocate process list\n"OSH_RESET);
        return 1;
    }

    /*
     * Initialize Variables For Executing Single OR Multiple Commands
     */
    size_t child_count = 0; // Used for keeping track of how many children were created
    size_t start = 0;       // Starting index Of Tokenized Array
    int previous_read = -1; // Previous pipe that stores the output of previously ran command
    pid_t final_child = -1; // Last child process created
    fflush(NULL);           // Flush out all output streams

    // Start Loop Through Tokenized Array
    while (tokens[start].token.type != TOKEN_EOF) {
        // Find The End Index For The First Command To Run
        size_t end = osh_find_command_end(tokens, start);
        // See If Next Token Needs The Output Of Previous Command
        int has_next = tokens[end].token.type == TOKEN_PIPE;
        // Create The Two Pipe File Descriptors
        int pipe_fd[2] = {-1, -1}; // Setting both descriptors to -1 says neither exist yet
        OSHCommand command = {0};  // Initialize for coverting tokenized array into an array

        // Build Up Command From Tokenized Array At 'start' Token Index
        if (osh_build_command(tokens, start, end, &command) != 0) {
            // If A Previous Command Was Ran Then Close Descriptor
            if (previous_read != -1) close(previous_read);
            // If Any Children Are Currently Active Then Wait For Them To Finish
            if (osh_wait_for_children(children, child_count, final_child) == -1) { free(children); return 1; }
            // Free The Children Process ID's After Waiting For Children
            free(children);
            return 1;
        }

        // If another command follows first command then create pipe
        if (has_next && pipe(pipe_fd) == -1) {
            // Free args, close discriptors and wait for any child processes previously created
            perror("osh: pipe");
            free(command.argv);
            if (previous_read != -1) close(previous_read);
            if (osh_wait_for_children(children, child_count, final_child) == -1) { free(children); return 1; }
            free(children);
            return 1;
        }

        // Create a New Process Image (Child Process)
        pid_t pid = fork();
        // If creating child process failed
        if (pid == -1) {
            // Free args, close discriptors and wait for any child processes previously created
            perror("osh: fork");
            free(command.argv);
            if (previous_read != -1) close(previous_read);
            if (pipe_fd[0] != -1) close(pipe_fd[0]);
            if (pipe_fd[1] != -1) close(pipe_fd[1]);
            if (osh_wait_for_children(children, child_count, final_child) == -1) { free(children); return 1; }
            free(children);
            return 1;
        }

        // Enter child process
        if (pid == 0) {
            // If any previous command ran, then grab that output as the new stdin for next command
            if (previous_read != -1 && dup2(previous_read, STDIN_FILENO) == -1) {
                perror("osh: dup2");
                // We are using _exit() since we are within the child process right now
                // We only want to clean up the child process, not the parent
                _exit(EXIT_FAILURE);
            }
            // Have stdout represent the write end of the pipe that both the parent and children can see
            if (has_next && dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
                perror("osh: dup2");
                _exit(EXIT_FAILURE);
            }
            // Apply redirection if needed
            // If needed then create the files needed and attach them to the file descriptors stdin/stdout
            if (osh_apply_redirections(&command) != 0) _exit(EXIT_FAILURE);

            // Close unused descriptors in the child
            if (previous_read != -1) close(previous_read);
            if (pipe_fd[0] != -1) close(pipe_fd[0]);
            if (pipe_fd[1] != -1) close(pipe_fd[1]);

            // Check to see if command is a shell builtin
            if (osh_is_builtin(command.argv[0])) {
                int builtin_status = osh_run_builtin(command.argv);
                // Clean out all child output streams
                fflush(NULL);
                // Exit child process whether successful or not
                _exit(builtin_status == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
            }

            // Execute normal program if not a shell builtin
            execvp(command.argv[0], command.argv);
            // Only runs if execvp returns an error
            perror("osh");
            // 127 is exit status for commands not found on Linux systems
            _exit(127);
        }

        // Add child's PID into an array and set as final_child if this so happens to be so
        children[child_count++] = pid;
        final_child = pid;
        // No longer need arguments
        free(command.argv);

        // Close anything descriptors that were previously open if needed
        if (previous_read != -1) close(previous_read);
        if (pipe_fd[1] != -1) close(pipe_fd[1]);
        // Check for any follow up commands and prepare index if so
        previous_read = has_next ? pipe_fd[0] : -1;
        start = has_next ? end + 1 : end;
    }

    // If there is a read-end file descriptor still open then close it
    if (previous_read != -1) close(previous_read);
    // Wait for all child processes that were created then free all the children PID's
    if (osh_wait_for_children(children, child_count, final_child) == -1) { free(children); return 1; }
    free(children);
    return 1;
}

// Find The End Index Of Command(s)
static size_t osh_find_command_end(const OSHToken *tokens, size_t start)
{
    // Pass Index Of 'start'
    size_t end = start;
    // Loop Until The Pipe Operator Or EOF Is Found; Adding 1 To 'end' Variable
    while (tokens[end].token.type != TOKEN_PIPE && tokens[end].token.type != TOKEN_EOF) end++;
    // Return End
    return end;
}

static int osh_validate_commands(const OSHToken *tokens)
{
    // Starting Index
    size_t start = 0;

    // Loop Through The 'tokens' Array
    while (tokens[start].token.type != TOKEN_EOF) {
        // Find 'end' Index For Command In The Array Before And After Pipe Operator
        size_t end = osh_find_command_end(tokens, start);
        size_t word_count = 0;

        // Check If 'end' Equals 'start'; If So Then Return Error For Misuse Of Pipe Operator
        if (end == start) {
            fprintf(stderr, OSH_ERROR"osh: expected a command beside '|'\n"OSH_RESET);
            return -1;
        }

        // Grab Command Before And After Pipe Operator If Pipe Operator Was Presented
        for (size_t i = start; i < end; i++) {
            // See If Token Is A Command/Argument
            if (tokens[i].token.type == TOKEN_WORD) {
                word_count++;
                // Then Continue Through To The Next Token Check
                continue;
            }
            // Check For Other Operators Excluding The Pipe Operator
            if (tokens[i].token.type == TOKEN_REDIRECT_IN
                || tokens[i].token.type == TOKEN_REDIRECT_OUT
                || tokens[i].token.type == TOKEN_APPEND_OUT) {
                // If Next Token Is Not Labled As 'TOKEN_WORD' Then We Know That The
                // User Did Not Provide A Filename Or Anything At All
                if (i + 1 >= end || tokens[i + 1].token.type != TOKEN_WORD) {
                    fprintf(stderr, OSH_ERROR"osh: redirection requires a filename\n"OSH_RESET);
                    return -1;
                }
                // Check Next Token If Operators Were Provided Properly
                i++;
                continue;
            }
            // If Anything Falls Through To The Bottom Of Loop Then We Know The Syntax Is Not Supported...
            // Yet
            fprintf(stderr, OSH_ERROR"osh: unsupported command syntax\n"OSH_RESET);
            return -1;
        }

        // If For Some Reason Something Falls Through That Previous Functions Didn't Catch
        // But This Should Never Be True
        if (word_count == 0) { fprintf(stderr, OSH_ERROR"osh: command section is missing a command\n"OSH_RESET); return -1; }

        // If Token Equals The Pipe Operator And The Next Token Equals EOF, Then We Know The Pipe Operator Was Misused
        if (tokens[end].token.type == TOKEN_PIPE
            && tokens[end + 1].token.type == TOKEN_EOF) {
            fprintf(stderr, OSH_ERROR"osh: expected a command after '|'\n"OSH_RESET);
            return -1;
        }
        // If End Is Currently Looking At The Pipe Token, Then Add One To Starting Index
        // Else Just Return End Where It Stands
        start = tokens[end].token.type == TOKEN_PIPE ? end + 1 : end;
    }
    // Return 0 If Successfully Identified Every Command
    return 0;
}

static int osh_build_command(const OSHToken *tokens, size_t start, size_t end,
                                OSHCommand *command)
{
    // Count For Amount Of Arguments Including The Command
    size_t argument_count = 0;
    // Start Checking For Every Tokenized Command/Argument
    for (size_t i = start; i < end; i++) {
        if (tokens[i].token.type == TOKEN_WORD) argument_count++;
    }

    // Heap For The Amount Of Arguments We Have Including The Command
    command->argv = malloc(sizeof(char *) * (argument_count + 1));
    // IF Malloc Failed
    if (!command->argv) {
        fprintf(stderr, OSH_ERROR"osh: malloc failed to allocate command arguments\n"OSH_RESET);
        return -1;
    }

    // Position Of The 'char **'
    size_t position = 0;
    // Grab Every Token And Place Them Into The Array
    for (size_t i = start; i < end; i++) {
        switch (tokens[i].token.type) {
            // Capture Command/Argument
            case TOKEN_WORD:
                command->argv[position++] = tokens[i].token.value;
                break;
            // Capture Filename From Next Token
            case TOKEN_REDIRECT_IN:
                command->input_path = tokens[++i].token.value;
                break;
            // Capture Filename From Next Token
            // Setting The Flags For Created File TO Overwrite
            case TOKEN_REDIRECT_OUT:
                command->output_path = tokens[++i].token.value;
                command->output_flags = O_WRONLY | O_CREAT | O_TRUNC;
                break;
            // Capture Filename From Next Token
            // Setting The Flags For File TO Append
            case TOKEN_APPEND_OUT:
                command->output_path = tokens[++i].token.value;
                command->output_flags = O_WRONLY | O_CREAT | O_APPEND;
                break;
            // Anything Else Gets Ignored
            default:
                break;
        }
    }
    // Set The End Of Arguments To Be NULL After Successfully Creating New Struct
    command->argv[position] = NULL;
    return 0;
}

// Prepare The Redirect Files For Use
static int osh_apply_redirections(const OSHCommand *command)
{
    // If Redirect-In Was Used Then Open The File To Read From
    if (command->input_path) {
        // Open File For Read-Only
        int input_fd = open(command->input_path, O_RDONLY);
        // If open() Failed
        if (input_fd == -1) { perror("osh"); return -1; }
        // Duplicate The Open File Descriptor And Check If dup2() Failed
        if (dup2(input_fd, STDIN_FILENO) == -1) { perror("osh: dup2"); close(input_fd); return -1; }
        // Close File Descriptor After Successfully Reading From A File
        close(input_fd);
    }

    // If Redirect-Out Was Used Then Create File
    if (command->output_path) {
        // 0666 is permission for read and write only to everyone
        int output_fd = open(command->output_path, command->output_flags, 0666);
        // If open() Fails
        if (output_fd == -1) { perror("osh"); return -1; }
        // Duplicate The Open File Descriptor And Check If dup2() Failed
        if (dup2(output_fd, STDOUT_FILENO) == -1) { perror("osh: dup2"); close(output_fd); return -1; }
        // Close File Descriptor After Successfully Creating Files
        close(output_fd);
    }
    return 0;
}

// Run Builtin While Aware Of Special Operators
static int osh_run_parent_builtin(OSHCommand *command)
{
    // Initialize Stdout And Stdin Status
    int saved_stdin = -1;
    int saved_stdout = -1;

    fflush(NULL); // Flush out all output streams
    // If Redirect-In Was Given
    if (command->input_path) {
        // Duplicates The File Descriptor 'STDIN_FILENO'
        // 'STDIN_FILENO' is just 0
        saved_stdin = dup(STDIN_FILENO);
        // If dup() Failed
        if (saved_stdin == -1) { perror("osh: dup"); return 1; }
    }
    // If Redirect-Out/Append Was Given
    if (command->output_path) {
        // Duplicates The File Descriptor 'STDOUT_FILENO'
        // 'STDOUT_FILENO' is just 1
        saved_stdout = dup(STDOUT_FILENO);
        // If dup() Failed
        if (saved_stdout == -1) {
            perror("osh: dup");
            // Close Copied File Descriptor If Previous dup() Failed
            if (saved_stdin != -1) close(saved_stdin);
            return 1;
        }
    }

    // Send 'command' To Apply The Redirection Logic
    if (osh_apply_redirections(command) != 0) {
        // Dup The Two Streams, Input and Output
        if (saved_stdin != -1) {
            // This Will Undo The First Calls To dup() For Stdin
            dup2(saved_stdin, STDIN_FILENO);
            // Then Close File Descriptor
            close(saved_stdin);
        }
        if (saved_stdout != -1) {
            // This Will Undo The First Calls To dup() For Stdout
            dup2(saved_stdout, STDOUT_FILENO);
            // Then Close File Descriptor
            close(saved_stdout);
        }
        return 1;
    }

    // Run The Builtin Command After Addressing Any File Descriptor Created From Special Operators
    // Only Handing The 'char **' Command/Argument(s)
    int status = osh_run_builtin(command->argv);
    // After Running Builtin, Flush All Output Streams
    fflush(NULL);

    // Move Current Input Stream Back Into Main Stdin File Descriptor
    if (saved_stdin != -1) {
        if (dup2(saved_stdin, STDIN_FILENO) == -1) perror("osh: dup2");
        close(saved_stdin);
    }
    // Move Current Output Stream Back Into Main Stdin File Descriptor
    if (saved_stdout != -1) {
        if (dup2(saved_stdout, STDOUT_FILENO) == -1) perror("osh: dup2");
        close(saved_stdout);
    }
    // If Builtin Was Not Found Then Return 1, Else Return 'status'
    return status == -1 ? 1 : status;
}

static int osh_wait_for_children(const pid_t *children, size_t child_count,
                                    pid_t final_child)
{
    // For Final Status Code
    int final_status = 0;
    int result = 0;

    // Loop Through Any Children That Were Created Before Current Process
    for (size_t i = 0; i < child_count; i++) {
        int status;
        pid_t waited;
        // Wait For The First Child If Still Not Finished
        do {
            // Wait For PID Of Every Child One At A Time Starting From The First Child Process
            waited = waitpid(children[i], &status, 0);
        // Stay Checking Until Child Exits Normally
        } while (waited == -1 && errno == EINTR);

        // If waitpid() Failed
        if (waited == -1) {
            perror("osh: waitpid");
            // Set Result To -1 And Continue Looping Through Each Child PID
            result = -1;
            continue;
        }
        // If We Reach The Final Child PID
        if (children[i] == final_child) {
            // If Child Exited Normally, By Calling exit() ,_exit(), Or By Returning To main()
            // Store Child Status Into 'final_status'
            if (WIFEXITED(status)) final_status = WEXITSTATUS(status);
            // If Child Was Terminated By A Signal
            // Store The Number Of The Signal That Terminated Child
            else if (WIFSIGNALED(status)) final_status = 128 + WTERMSIG(status);
        }
    }
    // Return The Result Of 'final_status' If Result Doesn't Hold An Error
    return result == -1 ? -1 : final_status;
}
