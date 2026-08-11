# OmniShell

OmniShell is an early Unix-style shell written in C. It combines the normal foundation of a small interactive shell—prompting, parsing, builtins, external command execution, and child-process waiting—with an experimental builtin called **OmniRun**.

OmniRun addresses a small annoyance from programming exercises and single-file experiments: repeatedly typing or remembering the interpreter or compiler command. Instead of entering `python3 test.py`, or compiling and then running a C file manually, OmniShell can select the current tool from the file extension.

This repository represents an early **v0.1-style foundation** and a systems-programming learning project. OmniShell is not presented as a replacement for Bash or Zsh, and OmniRun is not a replacement for Make, CMake, Meson, or another build system.


#### Video Demo:  https://youtu.be/x0x3imhMWIY


## Current Features

- Interactive REPL input through GNU Readline.
- In-session command history, the `history` builtin, and standard Readline editing keys.
- A current-directory prompt refreshed after every command.
- A compact startup identity banner with semantic terminal colors.
- Builtins for `cd`, `help`, `pwd`, `history`, `exit`, and `omnirun`.
- External command execution through the system `PATH` or an explicit path.
- OmniRun support for one explicit `.py`, `.c`, or `.cpp` source file.
- Child-process result tracking for normal exits, execution failures, signals, and process setup failures inside OmniRun.

## Requirements

OmniShell currently targets a Linux/POSIX(_POSIX_C_SOURCE 200809L) environment and uses POSIX process and filesystem interfaces including `fork()`, `execvp()`, `waitpid()`, `pipe()`, `fcntl()`, `stat()`, and `getcwd()`.

To build and use the current version, you need:

- A C17-capable compiler such as GCC.
- GNU Readline headers and library development files.
- A POSIX-compatible system environment.
- `python3` to run Python files through OmniRun.
- GCC for C compilation.
- G++ for C++ compilation through OmniRun.

On many Linux distributions the Readline dependency is provided by a package named `readline`, `readline-devel`, or `libreadline-dev`.

## Build

From the repository directory, create the OmniShell executable once with:

```sh
./omnicreate.sh
```

If `./omnishell` does not exist, the helper compiles all current translation units and links GNU Readline. After a successful build, it temporarily compiles and runs `welcome_to_omnishell.c` to show the OmniShell introduction, removes that temporary helper executable, and then prints the normal success and launch messages. The introduction is not shown when the OmniShell build fails.

If `./omnishell` already exists, the helper exits successfully without rebuilding, overwriting it, or showing the first-build introduction. Compiler and linker diagnostics remain visible intentionally, so a missing compiler, Readline development dependency, or source error is reported directly by the build tools.

After a successful build, start the shell with:

```sh
./omnishell
```

There is no Makefile in the repository yet. The equivalent direct build command is:

```sh
gcc -std=c17 -Wall -Wextra -Wpedantic \
    omnishell.c omnifunc.c omnibuiltins.c omnirun.c \
    -o omnishell -lreadline
```

## Testing

Run the v0.1 integration suite with:

```sh
./tests/run_tests.sh
```

The dependency-free Bash harness builds a separate test binary and creates all fixtures in an isolated temporary directory. It checks the one-time build helper and the major user-visible v0.1 paths: startup, builtins, custom history, external commands, Python/C/C++ OmniRun execution, output-collision protection, compiler-failure cleanup, and continued shell operation after errors. Each check reports `PASS` or `FAIL`, and the script returns a nonzero status if any test fails. See [`tests/README.md`](tests/README.md) for the testing approach.

## Start OmniShell

Run the compiled executable:

```sh
./omnishell
```

A new session initializes Readline, prints the OmniShell v0.1 banner once, and enters the read-evaluate-execute loop. The prompt shows the current working directory followed by the OmniShell identity. If a path contains more than five directory components, the current implementation shortens only the displayed prompt to an ellipsis plus the final three components. The real working directory is unchanged, and `pwd` still prints its complete absolute path.

Readline adds non-empty commands to its in-memory history. OmniShell also stores one copy of each accepted command for its own `history` builtin. Pressing EOF, normally `Ctrl+D`, leaves the loop.

## Builtins

Builtin names and function pointers are registered as matching entries in `omnibuiltins.c`.

### `cd`

```text
cd <directory>
cd
```

Changes OmniShell's own working directory. With no argument, `cd` uses the `HOME` environment variable. Because the change occurs inside the shell process, the next prompt reflects the new directory.

Examples:

```sh
cd /tmp
cd ..
cd
```

### `pwd`

```text
pwd
```

Prints the complete current working directory. Its dynamically allocated `getcwd()` buffer grows when the initial capacity is too small and is freed after use.

### `help`

```text
help
```

Prints the currently registered builtin names and a short usage introduction.

### `history`

```text
history
```

Prints accepted commands from the current OmniShell session in entry order with numbered lines. It accepts no additional arguments. GNU Readline still provides interactive line editing and its own in-session history support; OmniShell additionally maintains a fixed in-memory array of up to `OMNI_BUFSIZ` command copies for this builtin. When that capacity is reached, the oldest entry is freed and discarded before a new one is stored. History is not persisted between separate OmniShell launches.

### `exit`

```text
exit
```

Returns a false loop status and ends the current OmniShell session.

### `omnirun`

```text
omnirun <file>
```

Runs one supported, explicit, regular source file. OmniRun supplies the interpreter or compiler command; users should pass only the source path.

## External Commands

Commands not matched by the builtin table are launched as child processes. OmniShell passes the token array to `execvp()` and waits for the child before showing the next prompt.

Examples:

```sh
ls
gcc --version
./program
examples/program
```

`execvp()` searches `PATH` when the command contains no slash. A relative executable such as `./shell-c/test` begins from the current directory. An absolute-looking path such as `/shell-c/test` begins at the filesystem root and refers to a different location.

The current parser splits input on whitespace characters. It does not yet implement shell quoting, escaping, pipelines, redirection, glob expansion, background execution, or job control. Operator characters such as `|`, `>`, and `&` are currently ordinary arguments rather than shell syntax.

## OmniRun

### Purpose and Scope

OmniRun v0.1 is designed for small exercises, experiments, and single-file programs:

```sh
omnirun test.py
omnirun test.c
omnirun test.cpp
```

It saves repeatedly typing commands similar to:

```sh
python3 test.py

gcc test.c -o test
./test

g++ test.cpp -o test
./test
```

This is deliberately a one-file workflow. Version 0.1 accepts exactly one source-file argument and recognizes `.py`, `.c`, and `.cpp` from the final extension.

### Python

For a Python source:

```sh
omnirun foo.py
```

OmniRun currently selects `python3`, launches the equivalent of `python3 foo.py`, waits for the process result, reports a failure if needed, and then returns control to OmniShell.

### C

For a C source:

```sh
omnirun bar.c
```

OmniRun:

1. Confirms `bar.c` exists and is a regular file.
2. Derives the output path `bar` by removing only the final extension.
3. Refuses to continue if `bar` already exists.
4. Safely reserves the new output path.
5. Runs the equivalent of `gcc bar.c -o bar`.
6. Checks the compiler's actual process result.
7. Executes `./bar` only after successful compilation.
8. Returns control to OmniShell.

Directory portions and earlier dots are preserved:

```text
examples/bar.c -> examples/bar
foo.test.c     -> foo.test
```

After a successful compile and run, the executable remains available. If compilation fails, OmniRun removes the partial/reserved output that it safely created. It never intentionally removes or overwrites a pre-existing collision file.

### C++

`.cpp` files use `g++` and follow the same validation, output derivation, collision protection, compilation-result check, execution, and failed-build cleanup path as C. The automated suite verifies this with a program using `<iostream>` and `std::cout`, which requires normal C++ compilation and standard-library linking.

### Validation and Errors

OmniRun rejects:

- A missing source argument.
- More than one argument.
- User-supplied compiler/interpreter commands before the source.
- Missing paths.
- Directories and other non-regular files.
- Files without an extension.
- Unsupported extensions.
- A derived output path that already exists.

Compiler failure prevents program execution. OmniRun uses a close-on-exec pipe to distinguish an `execvp()` failure from a child program's own exit status, and records fork, wait, signal, and setup outcomes. Normal OmniRun errors return to the OmniShell prompt rather than terminating the shell.

## OmniRun Is Not a Build System

OmniRun v0.1 does not understand multiple translation units or arbitrary build options. It cannot currently replace commands such as:

```sh
gcc main.c parser.c utils.c -o program
gcc program.c -o program -lm -pthread
gcc omnishell.c omnifunc.c omnibuiltins.c omnirun.c -o omnishell -lreadline
```

It does not accept program arguments, multiple source files, include paths, library paths, preprocessor definitions, optimization flags, build manifests, or dependency graphs. Project/build-system detection, directory scanning, and automatic language detection outside the explicit `omnirun` builtin are also outside v0.1.

## Architecture

| File | Current responsibility |
| --- | --- |
| `omnishell.c` | Program entry point; starts the shell loop. |
| `omnishell.h` | Shared semantic terminal-color definitions. |
| `omnifunc.c` | Readline initialization, startup banner, prompt construction, input parsing, shell execution control, external process launching, and current-directory display. |
| `omnifunc.h` | Shared shell macros, dependencies, and public shell-control declarations. |
| `omnibuiltins.c` | Private builtin registration tables, dispatch, custom session-history storage, and the small `cd`, `help`, `pwd`, `history`, and `exit` implementations. |
| `omnibuiltins.h` | Public builtin declarations and OmniRun interface dependency. |
| `omnirun.c` | OmniRun validation, extension/tool selection, output naming, collision protection, compiler/interpreter execution, process-result handling, and cleanup. |
| `omnirun.h` | OmniRun's public declaration plus the process-result types and current preprocessing dependencies. |

The normal external-command path uses `fork()`, `execvp()`, and `waitpid()`. OmniRun adds a pipe marked close-on-exec so the parent can tell whether execution itself failed. Builtin functions return a loop status so errors can return control to the REPL.

## What This Project Demonstrates

OmniShell is a deliberate systems-programming learning project. Its implementation applies concepts from C, POSIX APIs, Linux process execution, child-process status handling, file descriptors, dynamic memory, command parsing, Readline, compiler/linker behavior, error propagation, and debugging.

The goal is to build and understand a reliable foundation, then use that foundation to explore practical shell features instead of presenting generated demo code or claiming production-shell completeness.

## Known Limitations

- Whitespace-only tokenization; no quote or backslash interpretation.
- No pipelines, redirection, globbing, background jobs, or job control.
- No persistent history file; the custom history builtin stores at most `OMNI_BUFSIZ` entries for the running session.
- External command status is not exposed as a shell variable.
- OmniRun accepts one source file and no program arguments.
- OmniRun supports only `.py`, `.c`, and `.cpp`.
- OmniRun does not accept custom compiler/interpreter commands or flags.
- OmniRun deliberately refuses to replace an existing derived output.
- No Makefile is currently tracked.

## Roadmap

### v0.1 finishing work

- Verify the final semantic colors and startup banner in an interactive terminal.
- Finish documentation and repository presentation review.

### v0.2 direction

- Add more common shell builtins.
- Support additional languages in OmniRun.
- Improve OmniRun language handling and user-facing feedback.
- Continue shell interface and usability polish.
- Improve process and error reporting where the current architecture supports it.

Version 0.2 is not a promise to solve arbitrary multi-file builds, dependency graphs, linker options, or build manifests.

### Longer-term possibilities

- Multiple C/C++ translation units.
- Explicit compiler, linker, include, library, and program arguments.
- More sophisticated project awareness or build metadata.
- Additional normal shell capabilities after the core interface is reliable.

## Author

Demetrius Jackson — OmniKing
