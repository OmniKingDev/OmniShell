#include "cell.h"

int main(int ac, char **av)
{
    (void)ac;
    int status;

    // Child Process
    if (fork() == 0)
        // Replace the current process image with a new process image
        // 'v' for vector, 'p' for path
        // Takes an array of arg(s) and uses PATH to find the executables
        // char **av = {'ls', '-la', NULL}; Also called flags
        // execvp('ls', av);
        execvp(av[1], av + 1);

    wait(&status);

    return EXIT_SUCCESS;
}
