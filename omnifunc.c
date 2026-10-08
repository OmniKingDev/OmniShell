// Holds All Language/POSIX Headers For Shell
#include "omnishell.h"

// All Other 'osh*' Made Functions
// More Info On Functions Are In 'osh*.c' Files
#include "omnifunc.h"
#include "omnilauncher.h"
#include "omnireadline.h"
#include "omniparser.h"

/* Prototypes That 'osh()' Uses */
//  NOTE: More Info On Functions Are Below
static char *osh_prompt_path(const char *current_dir);
static char *osh_complete_prompt_path(char *dir);
static void osh_print_banner(void);

// Used In omnishell.c To Launch Shell
// Main shell function
void osh(void)
{
    // Used For Prompt To Show User
    char *current_dir_prompt;

    // For Functions Later Used To Readline
    char *line;

    // Used To Take In Tokenized Entry
    OSHToken *argv;

    // Status Of Execution For Loop
    int status;

    // Initialize Readline's Interactive Mode Including Custom Keybinds
    osh_init_readline();

    // Initialize History Library Features
    osh_init_history();

    // Print Custom Banner
    // osh_print_banner();

    // Start Shell (REPL) Loop
    do {
        // Grab Absolute Path Of Current User Directory
        // Then Update The Directories To Limit What Is Shown To User
        // 'osh_cwd()' Grabs Absolute Path Of Current Working Directory
        // 'osh_complete_prompt_path()' Shortens The Path If It Exceeds Limit
        // Limit Is 5
        current_dir_prompt = osh_complete_prompt_path(osh_cwd());

        // Just In-case 'osh_complete_prompt_path()' Returns Nothing
        if (!current_dir_prompt) {
            fprintf(stderr, OSH_ERROR"osh: Function 'osh()':\nOsh failed to create prompt for Readline\n"OSH_RESET);
            free(current_dir_prompt);
            return;
        }

        // Then Hand Over Generated Prompt To Use In Custom 'osh_readline()' Function
        line = osh_readline(current_dir_prompt);

        // Free Prompt Once 'osh_readline()' Has Returned A 'line'
        // To Later Generate A New Prompt For User
        free(current_dir_prompt);

        // If 'osh_readline()' Returns NULL
        // Break Out Of Shell Loop
        if (!line) {
            fprintf(stderr, OSH_ERROR"osh: Function 'osh()':\nReadline failed to return a line\n"OSH_RESET);
            break;
        }

        // Lexical Analysis On Given Line
        argv = osh_tokenizer(line);

        // Execute Parsed Line Only Returning Status Of 'osh_execute()'
        // NOT The Line's Status Itself
        status = osh_execute(argv);

        // Free Up Memory Used To Recieve User Line
        free(line);

    // Check Status To Determine If Shell Continues Looping
    } while (status);

    // Upload Current Session History Into Persistent Shell File
    osh_end_history();
}


/*  NOTE:
 *  Functions To Build Directories Displayed For User
 *  Including Print Banner On Printed On Every Launch
 */

// Custom ASCII Banner To Print
static void osh_print_banner(void)
{
    printf(
OSH_BRPINK
" ▒█████   ███▄ ▄███▓ ███▄    █  ██▓  ██████  ██░ ██ ▓█████  ██▓     ██▓    \n"
"▒██▒  ██▒▓██▒▀█▀ ██▒ ██ ▀█   █ ▓██▒▒██    ▒ ▓██░ ██▒▓█   ▀ ▓██▒    ▓██▒    \n"
"▒██░  ██▒▓██    ▓██░▓██  ▀█ ██▒▒██▒░ ▓██▄   ▒██▀▀██░▒███   ▒██░    ▒██░    \n"
"▒██   ██░▒██    ▒██ ▓██▒  ▐▌██▒░██░  ▒   ██▒░▓█ ░██ ▒▓█  ▄ ▒██░    ▒██░    \n"
"░ ████▓▒░▒██▒   ░██▒▒██░   ▓██░░██░▒██████▒▒░▓█▒░██▓░▒████▒░██████▒░██████▒\n"
"░ ▒░▒░▒░ ░ ▒░   ░  ░░ ▒░   ▒ ▒ ░▓  ▒ ▒▓▒ ▒ ░ ▒ ░░▒░▒░░ ▒░ ░░ ▒░▓  ░░ ▒░▓  ░\n"
"  ░ ▒ ▒░ ░  ░      ░░ ░░   ░ ▒░ ▒ ░░ ░▒  ░ ░ ▒ ░▒░ ░ ░ ░  ░░ ░ ▒  ░░ ░ ▒  ░\n"
"░ ░ ░ ▒  ░      ░      ░   ░ ░  ▒ ░░  ░  ░   ░  ░░ ░   ░     ░ ░     ░ ░   \n"
"    ░ ░         ░            ░  ░        ░   ░  ░  ░   ░  ░    ░  ░    ░  ░\n"
OSH_BRPURPLE
"                         OmniShell v0.2                                    \n"
"             Welcome To The First Version Of OmniShell!!!                  \n"
OSH_RESET);
}

// Function For Current Working Directory
char *osh_cwd(void)
{
    // Using Custom Buf. Size
    size_t dir_size = DIR_BUFSIZ;

    // Heap Memory For Directory Path
    char *dir = malloc(sizeof(char) * dir_size);

    // If Malloc Failed
    if (!dir) {
        // Reference The Function Where Error Occured
        fprintf(stderr, OSH_ERROR "osh: Function 'osh_cwd()':\nMalloc failed to allocate memory\n" OSH_RESET);
        // Return No Current Working Directory
        return NULL;
    }

    // Grab User Current Working Directory
    while (getcwd(dir, dir_size) == NULL) {

        // If 'errno' Returns Anything Besides 'ERANGE'
        // Return Error Of Function Most Recently Used
        // Free Heap Memory
        // Return No Current Working Directory
        if (errno != ERANGE) { perror("osh"); free(dir); return NULL; }

        // If 'ERANGE' Returns As Error Than Resize Buff Of Directory Path
        char *resized_dir;

        // Add The Same Buff Size Into The Original Size, Doubling Size
        dir_size += DIR_BUFSIZ;

        // Realloc Heap Memory
        resized_dir = realloc(dir, sizeof(char) * dir_size);

        // If Realloc Failed
        if (!resized_dir) {

            // Reference The Function Where Error Occured
            fprintf(stderr, OSH_ERROR "osh: Function 'osh_cwd()':\nMalloc failed to allocate memory\n" OSH_RESET);

            // Free Heap Memory
            free(dir);

            // Return No Current Working Directory
            return NULL;
        }

        // Replace Original Heap With New Heap
        dir = resized_dir;
    }

    // Return Absolute Path Of User Current Working Directory
    return dir;
}

static char *osh_complete_prompt_path(char *dir)
{
    // For Current Directory User Is In
    char *display_dir;

    // Returns Shorter Version Of Current Working Directory Path
    // ONLY If Path Exceeds Set Limit For Directories Shown
    display_dir = osh_prompt_path(dir);

    // Just In-case 'osh_prompt_path()' Returns Nothing
    // Free Heap Memory
    // Exit Shell Entirely
    if (!display_dir) { free(dir); return NULL; }

    /* NOTE:
        *  All 'osh*' Made Functions Are Declared In Any Listed Header File At The Top
        *  EXCEPT For Any Function With A 'static' Type Remains In This File
    */

    // Build User Custom Prompt
    char *osh_prompt = "\n\001" OSH_BRPINK "\002omnishellv0.2\001" OSH_RESET "\002 ==> ";

    // Size Of Both Current Directory Path With Custom Prompt
    size_t prompt_size = strlen(display_dir) + strlen(osh_prompt) + 1;

    // Heap Memory For Prompt To Display To User
    char *prompt = malloc(prompt_size);

    // If Malloc Failed
    // Free Heap Memory
    // Exit Shell Entirely
    if (!prompt) { free(display_dir); free(dir); return NULL; }

    // Generate Final Prompt User Sees
    snprintf(prompt, prompt_size, "%s%s", display_dir, osh_prompt);

    // Free Heap Memory That Are No Longer Needed
    free(display_dir);
    free(dir);

    // Return Prompt For Readline To Use
    return prompt;
}

// Limit Amount Of Directories To Display
static char *osh_prompt_path(const char *current_dir)
{
    // To Keep Track Of Total Directories
    int components = 0;

    // Copy Absolute Directory
    // Path For Calculating Full Directory Length
    const char *position = current_dir;

    // Copy Absolute Directory Path
    // For Directories Being Returned
    const char *suffix = current_dir;

    // Set Limit Size
    size_t display_size;

    // Directory Path To Return
    char *display_dir;

    // Loop Until The NUL Byte Has Been Encountered
    while (*position != '\0') {

        // If Character Does NOT Equal '/' While The Character Before It Is '/'
        if (*position != '/' && position[-1] == '/') { components++; }

        // Look At Next Character In String
        position++;
    }

    // If Current Directory Length Exceeds 5 Directories
    if (components > 5) {

        // Limit For Directories To Display
        int DIR_LIMIT = 3;

        // Change Position To Point At The End Of Current Working Directory
        position = current_dir + strlen(current_dir);

        // Loop From The End To Start Of Current Working Directory
        while (position > current_dir) {

            // Start Stepping Back In String
            position--;

            // If '/' Is Found
            if (*position == '/') {

                // After Directory Is Found
                DIR_LIMIT--;

                // Once We Have 3 Directories
                // Suffix Should Hold Index At The First Character
                // Of The Third To Last Directory
                // Break Out Of While Loop
                if (DIR_LIMIT == 0) { suffix = position + 1; break; }
            }
        }

        // Length Of New Directory Path
        display_size = strlen(suffix) + 5;

        // Malloc Heap Memory
        display_dir = malloc(sizeof(char) * display_size);

        // If Malloc Failed
        // Reference The Function Where Error Occured
        // Return No Current Working Directory
        if (!display_dir) {
            fprintf(stderr, OSH_ERROR "osh: Function 'osh_prompt_path()':\nMalloc failed to allocate memory\n" OSH_RESET);
            return NULL;
        }

        // Copy New Directory Path With Ellipsis Reference
        snprintf(display_dir, display_size, ".../%s", suffix);

        // Return Full Directory Path
        return display_dir;
    }

    /* If File Path Components Is Less Than OR Equal To 5 */

    // Grab Length Of Directory Path
    display_size = strlen(current_dir) + 1;

    // Malloc Heap Memory
    display_dir = malloc(sizeof(char) * display_size);

    // If Malloc Failed
    // Reference The Function Where Error Occured
    // Return No Current Working Directory
    if (!display_dir) { fprintf(stderr, OSH_ERROR "osh: Function 'osh_prompt_path()':\nMalloc failed to allocate memory\n" OSH_RESET); return NULL; }

    // Copy Current Working Directory Into Malloced Heap Memory
    strcpy(display_dir, current_dir);

    // Return Full Directory Path
    return display_dir;
}
