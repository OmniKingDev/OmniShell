#!/usr/bin/env bash

# Build OmniShell once without replacing an existing executable.

# 'set' Built-In Bash Command Used To Change Behavior OR Internal Shell Options
# '-u' Is The 'nounset' Flag Option; Which Tells The Script To Exit
# If A Variable Is Used Before It Is Defined
set -u

# 'cd' Used To Change Directory Into The Directory Found By 'dirname'
# 'dirname' Gives Only The Absolute Path Of The Parent Directory Of Directory Given
# 'BASH_SOURCE' Is A Special Built-In Bash Array Variable
# That Holds The Path String Of The Script Being Executed; Always At Index 0
# After 'cd' Executed Successfully Then 'pwd' Grabs That Directory Absolute Path
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OMNISHELL="$SCRIPT_DIR/omnishell"
WELCOME_SOURCE="$SCRIPT_DIR/welcome_to_omnishell.c"
WELCOME_PROGRAM=""

# Create History File Used By GNU History Library Functions
HISTORY_FILE_FOR_SHELL="$HOME/.omnish_history"

# Function Made To Cleanup(Delete File(s)) Right Before Bash Exits
# No Matter How Bash Exits The Script
cleanup()
{
    # Checks For Conditional Expression
    # '-n' Checks To See If Variable's Length Is Greater Than 0
    if [[ -n "$WELCOME_PROGRAM" ]]; then
        # Delete File(s) Before Bash Exits
        rm -f -- "$WELCOME_PROGRAM"
    fi
}
# 'trap' Built-In Bash Utility Listens For Specific System Signals
# Or Shell Conditions To Trigger A Command
# Run 'cleanup' When 'trap' Catches Condition
# 'EXIT' Tells Script To Run 'cleanup' No Matter How It Closes
trap cleanup EXIT

# '-e' Checks To See If File Path Exist On File System
# '-L' Checks To See If Path Points To A Symbolic Link(Shortcut File)
# Tell The User The Shell Script Is Already Compiled
# The Script Will Not Attempt To Build The Requested Script(OMNISHELL)
if [[ -e "$OMNISHELL" || -L "$OMNISHELL" ]]; then
    printf 'OmniShell is already built. Run: ./omnishell\n'
    exit 0
fi
# OmniShell Compile Command To Run For User
clang -std=c17 -fsanitize=address -Wall -Wextra -Wpedantic \
    "$SCRIPT_DIR/omnishell.c" \
    "$SCRIPT_DIR/omnifunc.c" \
    "$SCRIPT_DIR/omniparser.c" \
    "$SCRIPT_DIR/omnilauncher.c" \
    "$SCRIPT_DIR/omnicommands.c" \
    "$SCRIPT_DIR/omnibuiltins.c" \
    "$SCRIPT_DIR/omnirun.c" \
    "$SCRIPT_DIR/omnireadline.c" \
    "$SCRIPT_DIR/omnireadline_keybinds.c" \
    -o "$OMNISHELL" -lreadline

# Grab Status Code Of Command Most Recently Ran('gcc')
status=$?

if (( "$status" == 0 )); then
    # 'mktemp' Creates A Random, Unique Temporary File Path
    # So Scripts Don't Overwrite Eachother
    # 'TMPDIR' Is Bash's Built-In Variable
    # If Nothing Is Withing The 'TMPDIR' Variable Then Use Fallback(/tmp)
    # 'XXXXXX' Is For 'mktemp' To Use To Ensure Path Uniqueness
    WELCOME_PROGRAM="$(mktemp "${TMPDIR:-/tmp}/welcome-to-omnishell.XXXXXX")"

    # Create The Welcome Program
    gcc -std=c17 -Wall -Wextra -Wpedantic \
        "$WELCOME_SOURCE" \
        -o "$WELCOME_PROGRAM"

    status=$?

    if [[ "$status" != 0 ]]; then
        printf 'OmniShell was built, but its welcome screen could not be created.\n' >&2
        exit "$status"
    fi

    # Print Welcome Prompt Then Notify OmniShell Has Been Compiled And Ready To Launch
    "$WELCOME_PROGRAM"
    printf 'OmniShell was created successfully.\n'
    printf 'Run it from the repository with: ./omnishell\n'

    if [[ -f "$HISTORY_FILE_FOR_SHELL" ]]; then
        echo 'Using previous ~/.omnish_history file since it already exist'
        exit 0
    else
        touch "$HISTORY_FILE_FOR_SHELL"
        echo 'Created Persistent History File ~/.omnish_history'
        echo 'Never REMOVE or DELETE this file'
        echo 'You must create the file manually if MISSING/LOST'
        echo "If NECESSARY, use the 'touch ~/.omnish_history' command to create the file"
        echo 'NOTE: All previous history would be lost by this point'
        echo "Or remove the compiled code 'rm -f omnishell' then re-run: ./omnicreate.sh to restore lost history file"
    fi

    exit 0
fi
# If Shell Could Not Compile
printf 'OmniShell could not be created. Review the compiler output above.\n' >&2
exit "$status"
