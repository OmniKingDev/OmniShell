# OmniShell

OmniShell is an early Unix-style shell written in C. It combines the normal foundation of a small interactive shell—prompting, parsing, builtins, external command execution, and child-process waiting—with an experimental builtin called **OmniRun**.

OmniRun addresses a small annoyance from programming exercises and single-file experiments: repeatedly typing or remembering the interpreter or compiler command. Instead of entering `python3 test.py`, or compiling and then running a C file manually, OmniShell can select the current tool from the file extension.

Version 0.2 established OmniShell's first completed foundation as a systems-programming learning project. The current branch contains development and verification work preparing the project for v0.3; v0.3 has not been formally released. OmniShell is not presented as a replacement for Bash or Zsh, and OmniRun is not a replacement for Make, CMake, Meson, or another build system.


#### Video Demo:  https://youtu.be/x0x3imhMWIY


## Current Features

- Interactive REPL input through GNU Readline, with command, pathname, and filename completion.
- Persistent GNU Readline/GNU History support, the `history` builtin, and standard Readline editing keys.
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

If `./omnishell` does not exist, the helper compiles all current translation units and links GNU Readline. After a successful build, it temporarily compiles and runs `welcome_to_omnishell.c`, cleans up that helper executable, and prints the normal success and launch messages. The successful first-build path also checks the OmniShell history file under the user's home directory. The introduction and history-file setup are not reached when the OmniShell build fails.

If `./omnishell` already exists, the helper exits successfully without rebuilding, overwriting it, or showing the first-build introduction. Compiler and linker diagnostics remain visible intentionally, so a missing compiler, Readline development dependency, or source error is reported directly by the build tools.

After a successful build, start the shell with:

```sh
./omnishell
```

There is no Makefile in the repository yet. The equivalent direct build command is:

```sh
gcc -std=c17 -Wall -Wextra -Wpedantic \
    omnishell.c omnifunc.c omniparser.c omnilauncher.c \
    omnicommands.c omnibuiltins.c omnirun.c \
    omnireadline.c omnireadline_keybinds.c \
    -o omnishell -lreadline
```

## Testing

Run the current integration suite with:

```sh
./tests/run_tests.sh
```

The Bash harness creates its fixtures in an isolated temporary directory and reports each check as `PASS` or `FAIL`. Its 36 passing integration tests cover the one-time build helper, startup, builtins, external commands, pipelines, redirection, malformed execution syntax, Python/C/C++ OmniRun execution, output-collision protection, compiler-failure cleanup, continued shell operation after errors, the GNU History lifecycle, and interactive Readline behavior.

Every shell test receives a temporary `HOME`, so production code naturally reads and writes a test-specific `.omnish_history` instead of the developer's real history file. History coverage verifies current-session ordering, existing-file loading, duplicate prevention, missing-file creation, cross-process persistence, deleted-file recovery, and invalid-argument handling. A small Python standard-library PTY driver sends real TAB and arrow-key sequences to verify interactive keybindings that piped input cannot exercise. See [`tests/README.md`](tests/README.md) for the testing approach.

## Start OmniShell

Run the compiled executable:

```sh
./omnishell
```

A new session initializes Readline, prints the OmniShell v0.1 banner once, and enters the read-evaluate-execute loop. The prompt shows the current working directory followed by the OmniShell identity. If a path contains more than five directory components, the current implementation shortens only the displayed prompt to an ellipsis plus the final three components. The real working directory is unchanged, and `pwd` still prints its complete absolute path.

OmniShell initializes GNU History and loads previously saved entries from `$HOME/.omnish_history` into memory. Each non-empty command accepted through Readline is added to that same in-memory history. OmniShell records the loaded history length at startup, then uses that boundary during normal shutdown to append only commands entered by the current process instead of duplicating older disk entries.

The history file does not require manual setup during normal shell use. If `$HOME/.omnish_history` is missing at startup, GNU History creates a clean OmniShell-specific file. If the file is deleted while OmniShell is running and the session has new commands to save, the failed append is followed by a full `write_history()` recovery from the history still held in memory.

### Readline Completion and Navigation

TAB completion treats the first word as a command. OmniShell searches its builtin names and executable regular files found through `PATH`, removes duplicate names, sorts matches alphabetically, and lets Readline display ambiguous results. When a result set exceeds 80 items, Readline asks before displaying the complete list. On a completely empty line, the first TAB rings the terminal bell and a second consecutive TAB requests the possible-completions display.

Arguments and command text containing `/` use Readline's normal filename/path completion instead of OmniShell's command-name search. Up and Down are bound to backward and forward prefix history search: text already entered on the line filters which history entries are visited. Ctrl-R continues to use GNU Readline's standard reverse-search behavior; OmniShell does not replace it with a custom search implementation.

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

Prints the current GNU History memory list in entry order with numbered lines, including entries loaded from earlier OmniShell sessions and commands accepted during the current session. It accepts no additional arguments. History persists through the OmniShell-specific `$HOME/.omnish_history` file; only the current session's new entries are appended during normal shutdown.

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

Commands not matched by the builtin table are launched as child processes. The launcher builds a borrowed-pointer argv for each pipe-delimited command section, calls `execvp()`, and waits for every child before showing the next prompt.

Examples:

```sh
ls
gcc --version
./program
examples/program
```

`execvp()` searches `PATH` when the command contains no slash. A relative executable such as `./shell-c/test` begins from the current directory. An absolute-looking path such as `/shell-c/test` begins at the filesystem root and refers to a different location.

The current tokenizer recognizes double-quoted word groups, pipelines, input redirection, output replacement, and output append. Examples include:

```sh
echo hello | grep hello
cat < input.txt
echo hello > output.txt
echo again >> output.txt
cat < input.txt | grep hello > result.txt
```

Standalone builtins run in the parent so commands such as `cd` and `exit` can change shell state. Builtins inside pipelines run in child processes. Single quotes, backslash escaping, glob expansion, background execution, and job control are not implemented.

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
gcc omnishell.c omnifunc.c omniparser.c omnilauncher.c omnicommands.c omnibuiltins.c omnirun.c omnireadline.c omnireadline_keybinds.c -o omnishell -lreadline
```

It does not accept program arguments, multiple source files, include paths, library paths, preprocessor definitions, optimization flags, build manifests, or dependency graphs. Project/build-system detection, directory scanning, and automatic language detection outside the explicit `omnirun` builtin are also outside v0.1.

## Architecture

| File | Current responsibility |
| --- | --- |
| `omnishell.c` | Program entry point; starts the shell loop. |
| `omnishell.h` | Shared semantic terminal-color definitions. |
| `omnifunc.c` | Shell loop, startup banner, prompt construction, and current-directory display. |
| `omnifunc.h` | Shared shell macros, dependencies, and public shell-control declarations. |
| `omniparser.c` | Tokenization, token locations, semantic classification, and token cleanup. |
| `omniparser.h` | Parser token types, metadata structures, and public parser declarations. |
| `omnilauncher.c` | Command-section argv preparation, builtin execution context, pipelines, redirection, fork/exec, descriptor ownership, and child waiting. |
| `omnilauncher.h` | Public execution entry points used by the shell loop. |
| `omnicommands.c` | Authoritative builtin names plus exact builtin and executable-command discovery. |
| `omnicommands.h` | Public shared-command discovery declarations and ownership contracts. |
| `omnibuiltins.c` | Private builtin function table, public dispatch boundary, and `cd`, `help`, `pwd`, `history`, `echo`, and `exit` implementations. |
| `omnibuiltins.h` | Public builtin declarations and dispatcher interface. |
| `omnireadline.c` | Readline initialization, input collection, GNU History initialization and persistence, and addition of accepted non-empty commands to history memory. |
| `omnireadline_keybinds.c` | Custom TAB completion plus Up/Down prefix-history keybindings. |
| `omnirun.c` | OmniRun validation, extension/tool selection, output naming, collision protection, compiler/interpreter execution, process-result handling, and cleanup. |
| `omnirun.h` | OmniRun's public declaration plus the process-result types and current preprocessing dependencies. |

The normal external-command path uses `fork()`, `execvp()`, and `waitpid()`. OmniRun adds a pipe marked close-on-exec so the parent can tell whether execution itself failed. Builtin functions return a loop status so errors can return control to the REPL.

## What This Project Demonstrates

OmniShell is a deliberate systems-programming learning project. Its implementation applies concepts from C, POSIX APIs, Linux process execution, child-process status handling, file descriptors, dynamic memory, command parsing, Readline, compiler/linker behavior, error propagation, and debugging.

The goal is to build and understand a reliable foundation, then use that foundation to explore practical shell features instead of presenting generated demo code or claiming production-shell completeness.

## Known Limitations

- Double quotes are recognized, but single quotes and backslash escaping are not interpreted.
- No globbing, background jobs, or job control.
- External command status is not exposed as a shell variable.
- OmniRun accepts one source file and no program arguments.
- OmniRun supports only `.py`, `.c`, and `.cpp`.
- OmniRun does not accept custom compiler/interpreter commands or flags.
- OmniRun deliberately refuses to replace an existing derived output.
- No Makefile is currently tracked.

## Roadmap

### Current development

- Review the completed GNU History persistence and interactive Readline integration coverage for the v0.2 release.
- Continue GNU Readline and interactive-shell usability work from this tested foundation.
- Continue development of common builtins and the one-time build helper.

### v0.2 release direction

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
