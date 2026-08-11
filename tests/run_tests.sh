#!/usr/bin/env bash

# OmniShell v0.1 uses integration tests: each test gives commands to the real
# shell and checks the user-visible result. An assertion is simply a comparison
# between what we expected and what the program actually produced.

set -u
set -o pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
TEST_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/omnishell-tests.XXXXXX")"
TEST_BINARY="$TEST_ROOT/omnishell-test"
TEST_WORKSPACE="$TEST_ROOT/workspace"
BUILD_LOG="$TEST_ROOT/build.log"

passed=0
failed=0
RUN_STATUS=0
RUN_OUTPUT=""
FAIL_REASON=""

# The temporary directory prevents tests from touching repository fixtures or
# the normal ./omnishell binary. trap runs cleanup even if the script is stopped.
cleanup()
{
    if [[ -n "$TEST_ROOT" && "$TEST_ROOT" == "${TMPDIR:-/tmp}/omnishell-tests."* ]]; then
        rm -rf -- "$TEST_ROOT"
    fi
}
trap cleanup EXIT

mkdir -p "$TEST_WORKSPACE/change-directory"

pass()
{
    printf '[PASS] %s\n' "$1"
    ((passed += 1))
}

fail()
{
    printf '[FAIL] %s\n' "$1"
    if [[ -n "$FAIL_REASON" ]]; then
        printf '       %s\n' "$FAIL_REASON"
    fi
    ((failed += 1))
}

# A test function returns status 0 for success and nonzero for failure. The
# counters create the final summary, and the script itself follows the same
# convention: 0 means every test passed; nonzero means at least one failed.
run_test()
{
    local name=$1
    local test_function=$2

    FAIL_REASON=""
    if "$test_function"; then
        pass "$name"
    else
        fail "$name"
    fi
}

# OmniShell writes useful information to both stdout and stderr, so each run
# captures both streams. ANSI colors and Readline control sequences are removed
# only from the captured copy so plain-text assertions remain readable.
strip_terminal_codes()
{
    sed $'s/\033\\[[0-9;?]*[[:alpha:]]//g' "$1" | tr -d '\r'
}

run_shell()
{
    local name=$1
    local commands=$2
    local raw_output="$TEST_ROOT/$name.raw"
    local clean_output="$TEST_ROOT/$name.txt"

    (
        cd "$TEST_WORKSPACE" || exit 1
        "$TEST_BINARY" <<< "$commands"
    ) >"$raw_output" 2>&1
    RUN_STATUS=$?

    strip_terminal_codes "$raw_output" >"$clean_output"
    RUN_OUTPUT="$(<"$clean_output")"
}

assert_status()
{
    local expected=$1

    if [[ "$RUN_STATUS" -ne "$expected" ]]; then
        FAIL_REASON="expected status $expected, received $RUN_STATUS"
        return 1
    fi
}

assert_contains()
{
    local expected=$1

    if [[ "$RUN_OUTPUT" != *"$expected"* ]]; then
        FAIL_REASON="output did not contain: $expected"
        return 1
    fi
}

assert_not_contains()
{
    local unexpected=$1

    if [[ "$RUN_OUTPUT" == *"$unexpected"* ]]; then
        FAIL_REASON="output unexpectedly contained: $unexpected"
        return 1
    fi
}

assert_line_present()
{
    local expected=$1

    if ! printf '%s\n' "$RUN_OUTPUT" | grep -Fqx -- "$expected"; then
        FAIL_REASON="output did not contain the complete line: $expected"
        return 1
    fi
}

assert_line_once()
{
    local expected=$1
    local matches

    matches=$(printf '%s\n' "$RUN_OUTPUT" | grep -Fxc -- "$expected")
    if [[ "$matches" -ne 1 ]]; then
        FAIL_REASON="expected one numbered history line '$expected', found $matches"
        return 1
    fi
}

test_build()
{
    if gcc -std=c17 -Wall -Wextra -Wpedantic \
        "$PROJECT_ROOT/omnishell.c" \
        "$PROJECT_ROOT/omnifunc.c" \
        "$PROJECT_ROOT/omnibuiltins.c" \
        "$PROJECT_ROOT/omnirun.c" \
        -o "$TEST_BINARY" -lreadline 2>"$BUILD_LOG"; then
        if [[ -s "$BUILD_LOG" ]]; then
            printf '%s\n' '--- compiler warnings ---'
            cat "$BUILD_LOG"
            printf '%s\n' '--- end compiler warnings ---'
        fi
        return 0
    fi

    cat "$BUILD_LOG"
    FAIL_REASON="compiler returned a nonzero status"
    return 1
}

test_create_helper()
{
    local bootstrap="$TEST_ROOT/bootstrap"
    local failed_bootstrap="$TEST_ROOT/failed-bootstrap"
    local expected_welcome_program="$TEST_ROOT/expected-welcome"
    local expected_welcome
    local first_output
    local second_output
    local failed_output
    local failed_status
    local before
    local after

    mkdir -p "$bootstrap/tmp" "$failed_bootstrap"
    cp "$PROJECT_ROOT/omnicreate.sh" "$bootstrap/"
    cp "$PROJECT_ROOT/omnishell.c" "$PROJECT_ROOT/omnishell.h" "$bootstrap/"
    cp "$PROJECT_ROOT/omnifunc.c" "$PROJECT_ROOT/omnifunc.h" "$bootstrap/"
    cp "$PROJECT_ROOT/omnibuiltins.c" "$PROJECT_ROOT/omnibuiltins.h" "$bootstrap/"
    cp "$PROJECT_ROOT/omnirun.c" "$PROJECT_ROOT/omnirun.h" "$bootstrap/"
    cp "$PROJECT_ROOT/welcome_to_omnishell.c" "$bootstrap/"

    if ! gcc -std=c17 -Wall -Wextra -Wpedantic \
        "$bootstrap/welcome_to_omnishell.c" \
        -o "$expected_welcome_program"; then
        FAIL_REASON="developer test setup could not compile the current welcome source"
        return 1
    fi
    expected_welcome="$("$expected_welcome_program")"
    if [[ -z "$expected_welcome" ]]; then
        FAIL_REASON="developer test setup received empty output from the welcome source"
        return 1
    fi

    if [[ -e "$bootstrap/omnishell" ]]; then
        FAIL_REASON="isolated bootstrap unexpectedly started with an omnishell executable"
        return 1
    fi

    first_output=$(cd "$bootstrap" && TMPDIR="$bootstrap/tmp" ./omnicreate.sh 2>&1)
    if [[ $? -ne 0 || ! -x "$bootstrap/omnishell" ]]; then
        FAIL_REASON="omnicreate.sh did not produce the isolated test executable"
        return 1
    fi

    before=$(cksum "$bootstrap/omnishell")
    second_output=$(cd "$bootstrap" && TMPDIR="$bootstrap/tmp" ./omnicreate.sh 2>&1)
    after=$(cksum "$bootstrap/omnishell")

    if [[ "$second_output" != *"already built"* ]]; then
        FAIL_REASON="second omnicreate.sh invocation did not report the existing build"
        return 1
    fi
    if [[ "$before" != "$after" ]]; then
        FAIL_REASON="second omnicreate.sh invocation changed the existing executable"
        return 1
    fi
    if [[ "$first_output" != *"created successfully"* ]]; then
        FAIL_REASON="first omnicreate.sh invocation did not report successful creation"
        return 1
    fi
    if [[ "$first_output" != *"$expected_welcome"* ]]; then
        FAIL_REASON="first omnicreate.sh invocation did not print the current welcome source output; this is a developer-side integration error"
        return 1
    fi
    if [[ "$second_output" == *"$expected_welcome"* ]]; then
        FAIL_REASON="second omnicreate.sh invocation unexpectedly printed the welcome screen"
        return 1
    fi
    if find "$bootstrap/tmp" -mindepth 1 -print -quit | grep -q .; then
        FAIL_REASON="omnicreate.sh left a temporary welcome executable behind"
        return 1
    fi

    cp "$PROJECT_ROOT/omnicreate.sh" "$failed_bootstrap/"
    cp "$PROJECT_ROOT/welcome_to_omnishell.c" "$failed_bootstrap/"
    failed_output=$(cd "$failed_bootstrap" && ./omnicreate.sh 2>&1)
    failed_status=$?

    if [[ "$failed_status" -eq 0 ]]; then
        FAIL_REASON="omnicreate.sh returned success after the OmniShell build failed"
        return 1
    fi
    if [[ "$failed_output" == *"$expected_welcome"* ]]; then
        FAIL_REASON="omnicreate.sh printed the welcome screen after a failed build"
        return 1
    fi
}

test_start_and_exit()
{
    run_shell start_exit $'exit'
    assert_status 0 && assert_contains "OmniShell v0.1"
}

test_pwd()
{
    run_shell pwd $'pwd\nexit'
    assert_status 0 && assert_line_present "$TEST_WORKSPACE"
}

test_cd_and_pwd()
{
    run_shell cd_pwd $'cd change-directory\npwd\nexit'
    assert_status 0 && assert_line_present "$TEST_WORKSPACE/change-directory"
}

test_help()
{
    run_shell help $'help\nexit'
    assert_status 0 \
        && assert_contains " cd" \
        && assert_contains " help" \
        && assert_contains " exit" \
        && assert_contains " pwd" \
        && assert_contains " omnirun" \
        && assert_contains " history"
}

test_history()
{
    run_shell history $'pwd\nhelp\nhistory\nexit'
    assert_status 0 \
        && assert_line_once "  1  pwd" \
        && assert_line_once "  2  help" \
        && assert_line_once "  3  history"
}

test_history_wrong_usage()
{
    run_shell history_usage $'history extra\nprintf HISTORY_SURVIVED\n\nexit'
    assert_status 0 \
        && assert_contains "Usage: history" \
        && assert_contains "HISTORY_SURVIVED"
}

test_external_command()
{
    run_shell external $'printf EXTERNAL_TEST_OK\nexit'
    assert_status 0 && assert_contains "EXTERNAL_TEST_OK"
}

test_omnirun_python()
{
    run_shell python $'omnirun python_case.py\nexit'
    assert_status 0 \
        && assert_contains "PYTHON_TEST_OK" \
        && [[ ! -e "$TEST_WORKSPACE/python_case" ]]
}

test_omnirun_c()
{
    run_shell c $'omnirun c_case.c\nexit'
    assert_status 0 \
        && assert_contains "C_TEST_OK" \
        && [[ -x "$TEST_WORKSPACE/c_case" ]]
}

test_omnirun_cpp()
{
    run_shell cpp $'omnirun cpp_case.cpp\nexit'
    assert_status 0 \
        && assert_contains "CPP_TEST_OK" \
        && [[ -x "$TEST_WORKSPACE/cpp_case" ]]
}

test_existing_output_protection()
{
    local before
    local after

    before=$(cksum "$TEST_WORKSPACE/collision")
    run_shell collision $'omnirun collision.c\nexit'
    after=$(cksum "$TEST_WORKSPACE/collision")

    assert_status 0 \
        && assert_contains "already exists; refusing to overwrite it" \
        && [[ "$before" == "$after" ]]
}

test_failed_compilation_cleanup()
{
    run_shell broken $'omnirun broken.c\nprintf COMPILE_ERROR_SURVIVED\nexit'
    assert_status 0 \
        && assert_contains "gcc exited with status" \
        && assert_contains "COMPILE_ERROR_SURVIVED" \
        && assert_not_contains "BROKEN_PROGRAM_RAN" \
        && [[ ! -e "$TEST_WORKSPACE/broken" ]]
}

test_unsupported_extension()
{
    run_shell unsupported $'omnirun program.xyz\nexit'
    assert_status 0 && assert_contains "unsupported file extension '.xyz'"
}

test_missing_source()
{
    run_shell missing $'omnirun does-not-exist.py\nprintf MISSING_SOURCE_SURVIVED\nexit'
    assert_status 0 \
        && assert_contains "cannot access 'does-not-exist.py'" \
        && assert_contains "MISSING_SOURCE_SURVIVED"
}

# Arrange: create isolated source files used by the black-box OmniRun tests.
cat >"$TEST_WORKSPACE/python_case.py" <<'PYTHON'
print("PYTHON_TEST_OK")
PYTHON

cat >"$TEST_WORKSPACE/c_case.c" <<'C_SOURCE'
#include <stdio.h>

int main(void)
{
    puts("C_TEST_OK");
    return 0;
}
C_SOURCE

cat >"$TEST_WORKSPACE/collision.c" <<'COLLISION_C'
int main(void)
{
    return 0;
}
COLLISION_C

printf 'PREEXISTING_OUTPUT_MUST_REMAIN\n' >"$TEST_WORKSPACE/collision"

# std::cout requires C++ compilation and linking, so this catches a regression
# where OmniRun accidentally sends .cpp files through gcc instead of g++.
cat >"$TEST_WORKSPACE/cpp_case.cpp" <<'CPP_SOURCE'
#include <iostream>

int main()
{
    std::cout << "CPP_TEST_OK\n";
    return 0;
}
CPP_SOURCE

cat >"$TEST_WORKSPACE/broken.c" <<'BROKEN_C'
#error INTENTIONAL_COMPILER_FAILURE

#include <stdio.h>

int main(void)
{
    puts("BROKEN_PROGRAM_RAN");
    return 0;
}
BROKEN_C

printf 'unsupported fixture\n' >"$TEST_WORKSPACE/program.xyz"

run_test "shell builds" test_build
run_test "one-time build helper" test_create_helper

# The remaining tests require the compiled test binary.
if [[ -x "$TEST_BINARY" ]]; then
    run_test "shell starts and exits" test_start_and_exit
    run_test "pwd builtin" test_pwd
    run_test "cd changes shell directory" test_cd_and_pwd
    run_test "help lists current builtins" test_help
    run_test "history builtin ordering" test_history
    run_test "history rejects arguments" test_history_wrong_usage
    run_test "external command" test_external_command
    run_test "omnirun Python" test_omnirun_python
    run_test "omnirun C" test_omnirun_c
    run_test "omnirun C++ through g++ behavior" test_omnirun_cpp
    run_test "existing output protection" test_existing_output_protection
    run_test "failed compilation cleanup and shell survival" test_failed_compilation_cleanup
    run_test "unsupported extension" test_unsupported_extension
    run_test "missing source and shell survival" test_missing_source
fi

printf '\n%d passed\n%d failed\n' "$passed" "$failed"

if [[ "$failed" -ne 0 ]]; then
    exit 1
fi
exit 0
