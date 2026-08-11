#!/usr/bin/env bash

# Build OmniShell once without replacing an existing executable.

set -u

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OMNISHELL="$SCRIPT_DIR/omnishell"
WELCOME_SOURCE="$SCRIPT_DIR/welcome_to_omnishell.c"
WELCOME_PROGRAM=""

cleanup()
{
    if [[ -n "$WELCOME_PROGRAM" ]]; then
        rm -f -- "$WELCOME_PROGRAM"
    fi
}
trap cleanup EXIT

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
    WELCOME_PROGRAM="$(mktemp "${TMPDIR:-/tmp}/welcome-to-omnishell.XXXXXX")"
    gcc -std=c17 -Wall -Wextra -Wpedantic \
        "$WELCOME_SOURCE" \
        -o "$WELCOME_PROGRAM"
    status=$?

    if [[ "$status" -ne 0 ]]; then
        printf 'OmniShell was built, but its welcome screen could not be created.\n' >&2
        exit "$status"
    fi

    "$WELCOME_PROGRAM"
    printf 'OmniShell was created successfully.\n'
    printf 'Run it from the repository with: ./omnishell\n'
    exit 0
fi

printf 'OmniShell could not be created. Review the compiler output above.\n' >&2
exit "$status"
