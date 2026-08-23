# OmniShell Integration Tests

These are primarily integration, or black-box, tests. They run the complete OmniShell program and check its visible output and filesystem effects instead of calling individual C functions directly.

Run the suite from anywhere inside the repository with:

```sh
./tests/run_tests.sh
```

You can also run it explicitly through Bash:

```sh
bash tests/run_tests.sh
```

The script creates an isolated temporary workspace, builds a separate `omnishell-test` binary, generates small Python, C, and C++ fixtures, and removes that workspace when it exits. It also copies the source and `omnicreate.sh` into that workspace to verify the one-time bootstrap and successful welcome-screen behavior without touching the repository's normal `omnishell` executable or existing source fixtures.

## Isolated History Testing

Each `run_shell` call sets `HOME` to a temporary directory named from that test, such as `$TEST_ROOT/home-history_file_recovery`. Production OmniShell code therefore uses that directory's `.omnish_history`; the developer's real `$HOME/.omnish_history` is never read, changed, or removed by the suite.

Different test names normally receive separate fake homes and clean history state. A persistence test intentionally reuses the same test name so two separate OmniShell processes share one fake home, allowing the second process to load history written by the first.

Terminal assertions inspect captured output after ANSI and Readline control sequences are removed. File assertions inspect the raw temporary `.omnish_history` directly, including exact line counts used to detect duplicated entries.

The suite exercises GNU History as real integration behavior rather than mocking it. Current history coverage verifies:

- Commands from the current session remain ordered in the `history` builtin.
- Existing `.omnish_history` entries load into GNU History memory.
- Previously loaded disk entries are not appended again as duplicates.
- A missing history file is created automatically.
- History persists across separate OmniShell processes.
- History is recreated from memory if its file is deleted during a running session.
- Invalid `history` arguments report an error without terminating OmniShell.

Each assertion compares an expected result with actual output, process status, or filesystem state. Every test prints `PASS` or `FAIL`; the final counters summarize the run. The current suite completes with `21 passed` and `0 failed`. The script exits with status `0` only when all tests pass, allowing it to be used by another script or future continuous-integration system.

OmniShell uses a dependency-free Bash harness because its most important behavior crosses the shell, builtin, compiler, interpreter, filesystem, history, and child-process boundaries. This keeps the arrange, execute, capture, and compare steps readable while the project is still small. A C framework such as Munit may become useful later when OmniShell has more isolated pure/helper functions that benefit from direct unit tests.
