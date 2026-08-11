# OmniShell v0.1 Tests

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

Each assertion compares an expected result with actual output, process status, or filesystem state. Every test prints `PASS` or `FAIL`; the final counters summarize the run. The script exits with status `0` only when all tests pass, allowing it to be used by another script or future continuous-integration system.

Version 0.1 uses a dependency-free Bash harness because its most important behavior crosses the shell, builtin, compiler, interpreter, and child-process boundaries. This keeps the arrange, execute, capture, and compare steps readable while the project is still small. A C framework such as Munit may become useful later when OmniShell has more isolated pure/helper functions that benefit from direct unit tests.
