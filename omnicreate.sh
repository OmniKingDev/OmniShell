#!/usr/bin/env bash

# Build OmniShell once without replacing an existing executable.

set -u

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OMNISHELL="$SCRIPT_DIR/omnishell"

if [[ -e "$OMNISHELL" || -L "$OMNISHELL" ]]; then
    printf 'OmniShell is already built. Run: ./omnishell\n'
    exit 0
fi

gcc -std=c17 -Wall -Wextra -Wpedantic \
    "$SCRIPT_DIR/omnishell.c" \
    "$SCRIPT_DIR/omnifunc.c" \
    "$SCRIPT_DIR/omnibuiltins.c" \
    "$SCRIPT_DIR/omnirun.c" \
    -o "$OMNISHELL" -lreadline
status=$?

if [[ "$status" -eq 0 ]]; then
    printf 'OmniShell was created successfully.\n'
    printf 'Run it from the repository with: ./omnishell\n'
    exit 0
fi

printf 'OmniShell could not be created. Review the compiler output above.\n' >&2
exit "$status"
