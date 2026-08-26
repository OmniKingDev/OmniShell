# OmniShell Integration Tests

These are integration, or black-box, tests. They run the complete OmniShell program and check its visible output, editable-line behavior, and filesystem effects instead of calling individual C functions directly.

Run the suite from anywhere inside the repository with:

```sh
./tests/run_tests.sh
```

You can also run it explicitly through Bash:

```sh
bash tests/run_tests.sh
```

The script creates an isolated temporary workspace, builds a separate `omnishell-test` binary, generates small Python, C, and C++ fixtures, and removes that workspace when it exits. It also copies the source and `omnicreate.sh` into that workspace to verify the one-time bootstrap and successful welcome-screen behavior without touching the repository's normal `omnishell` executable or existing source fixtures.

## Two Integration Paths

Ordinary command, builtin, OmniRun, and persistent-history cases feed lines through standard input. That is the smallest readable way to test behavior that does not depend on terminal key sequences.

Interactive Readline cases use `readline_pty_tests.py`, a Python standard-library driver built with `pty`, `select`, and related operating-system modules. It starts the real compiled shell in a pseudo-terminal and sends actual TAB, Up-arrow, and Down-arrow bytes. A PTY matters because piped input does not make Readline perform terminal editing or invoke its interactive keybindings. The helper uses polling and deadlines so a regression fails with the sent input and observed terminal output instead of hanging the suite.

Each PTY case has its own temporary working directory, `HOME`, and controlled `PATH`. Distinctive executable and file fixtures make completion results independent of most commands installed on the developer's machine. The interactive coverage verifies:

- Unique executable-command and builtin completion by submitting the completed line.
- Alphabetical display of ambiguous command matches.
- Suppression of the same executable name found in multiple `PATH` directories.
- The first empty-line TAB bell and second-TAB possible-completions display.
- Readline's over-80-match confirmation prompt and cancellation with `n` without Enter.
- Filename completion for an argument and pathname completion for a slash-containing command.
- Backward and forward prefix-history navigation using real arrow-key sequences.

## Isolated History Testing

Each `run_shell` call sets `HOME` to a temporary directory named from that test, such as `$TEST_ROOT/home-history_file_recovery`. Production OmniShell code therefore uses that directory's `.omnish_history`; the developer's real `$HOME/.omnish_history` is never read, changed, or removed by the suite.

Different test names normally receive separate fake homes and clean history state. A persistence test intentionally reuses the same test name so two separate OmniShell processes share one fake home, allowing the second process to load history written by the first.

Terminal assertions inspect captured output after ANSI and Readline control sequences are removed. File assertions inspect the raw temporary `.omnish_history` directly, including exact line counts used to detect duplicated entries.

The suite exercises GNU History as real integration behavior rather than mocking it. Current history lifecycle coverage verifies:

- Commands from the current session remain ordered in the `history` builtin.
- Existing `.omnish_history` entries load into GNU History memory.
- Previously loaded disk entries are not appended again as duplicates.
- A missing history file is created automatically.
- History persists across separate OmniShell processes.
- History is recreated from memory if its file is deleted during a running session.
- Invalid `history` arguments report an error without terminating OmniShell.

Each assertion compares an expected result with actual output, submitted-line behavior, process status, terminal bytes, or filesystem state. Every test prints `PASS` or `FAIL`; the final counters summarize the run. The current suite completes with `30 passed` and `0 failed`. The script exits with status `0` only when all tests pass, allowing it to be used by another script or future continuous-integration system.

OmniShell uses a Bash harness plus a Python standard-library PTY helper because its most important behavior crosses the terminal, shell, builtin, compiler, interpreter, filesystem, history, and child-process boundaries. No third-party test framework is required. This keeps the arrange, execute, capture, and compare steps readable while the project is still small. A C framework such as Munit may become useful later when OmniShell has more isolated pure/helper functions that benefit from direct unit tests.
