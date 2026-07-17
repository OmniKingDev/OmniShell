#include "cell.h"

// Prototype Of Function
char *cell_read_line(void);


int main(void)
{
    /*
    // REPL
    // Read
    // Evaluate
    // Print/Execute
    // Loop
    */

    // Create A Pointer Of Char Type
    char *line;

    // Banner Of Shell
    printbanner();

    // Starting Prompt Loop
    // 1) Present Prompt Get Line From CLI
    while (line = cell_read_line()) {

      // 2) Parse Line -- Get Tokens With gettok Function
      /* ( --> Lexing --> Parsing ) Is Proper Evaluating */
      
 
      // 3) Print Result/Execute child process
 
      // 4) Present A New Prompt/Loop To Start
      /* Which Is The Starting While Loop ( while (true) ) */
    }
    return EXIT_SUCCESS;
}

// Create A Function To Read Line
char *cell_read_line(void) {
    // Create A Buffer Of Char Type
    char *buff;

    // 'size_t' Means Unsigned Integer
    size_t buffsize;

    // 'Change Working Directory' Buffer
    char cwd[BUFSIZ];

    // Set 'buff' To NULL So 'getline' Can Allocate Memory On It's Own
    buff = NULL; // set to NULL

    // Using Wrapper implemented in 'cell.h'
    Getcwd(cwd, sizeof(cwd));
    if (isatty(fileno(stdin)) == 1) {
        p(C"😈 %s 😈"RST"\n∞∞∞▻", cwd);
    }

    // Call 'getline' Giving Us A Copy Of Stream From Console
    if (getline(&buff, &buffsize, stdin) == -1)
    {
        buff = NULL;
        // Returns A Non-Zero Value If Set
        if (feof(stdin))
            p(RED"[EOF]\n"RST); // 'RED' is color and RST is fallback color
        // Getline Failed Otherwise
        else
            p(RED"Getline Failed\n"RST);
    }
    return buff;
}
