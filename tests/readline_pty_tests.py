#!/usr/bin/env python3

"""Drive OmniShell through a PTY to test real GNU Readline behavior."""

import argparse
import fcntl
import os
import pty
import select
import signal
import struct
import sys
import time
import termios
from pathlib import Path


PROMPT_END = b" ==> "
READ_TIMEOUT = 4.0


class TestFailure(Exception):
    """A user-visible Readline behavior did not match the expectation."""


class ShellSession:
    def __init__(self, binary, workspace, home, path):
        self.sent = []
        self.observed = bytearray()
        pid, fd = pty.fork()
        if pid == 0:
            os.chdir(workspace)
            environment = os.environ.copy()
            environment.update(
                {
                    "HOME": str(home),
                    "PATH": path,
                    "TERM": "xterm",
                    "LC_ALL": "C",
                }
            )
            os.execve(binary, [binary], environment)

        self.pid = pid
        self.fd = fd
        # A stable terminal width keeps completion columns deterministic.
        fcntl.ioctl(fd, termios.TIOCSWINSZ, struct.pack("HHHH", 40, 160, 0, 0))
        self.read_until(PROMPT_END)

    def send(self, label, data):
        self.sent.append((label, data))
        os.write(self.fd, data)

    def _read_once(self, timeout):
        ready, _, _ = select.select([self.fd], [], [], timeout)
        if not ready:
            return b""
        try:
            chunk = os.read(self.fd, 4096)
        except OSError as error:
            # Linux PTYs normally report EIO after the child closes its side.
            if error.errno == 5:
                return b""
            raise
        self.observed.extend(chunk)
        return chunk

    def read_until(self, marker, timeout=READ_TIMEOUT):
        output = bytearray()
        deadline = time.monotonic() + timeout
        while marker not in output:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TestFailure(f"timed out waiting for {marker!r}")
            output.extend(self._read_once(remaining))
        return bytes(output)

    def read_until_idle(self, timeout=READ_TIMEOUT, idle=0.20):
        output = bytearray()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            chunk = self._read_once(idle)
            if not chunk:
                break
            output.extend(chunk)
        return bytes(output)

    def submit(self, line):
        self.send("submit line", line + b"\n")
        return self.read_until(PROMPT_END)

    def close(self):
        if getattr(self, "pid", None) is None:
            return
        try:
            # Clear any unfinished completion text, then leave through the builtin.
            self.send("clear line and exit", b"\x15exit\n")
            deadline = time.monotonic() + READ_TIMEOUT
            while time.monotonic() < deadline:
                pid, _ = os.waitpid(self.pid, os.WNOHANG)
                if pid == self.pid:
                    self.pid = None
                    return
                self._read_once(0.05)
        except (BrokenPipeError, OSError):
            pass

        if self.pid is not None:
            os.kill(self.pid, signal.SIGTERM)
            os.waitpid(self.pid, 0)
            self.pid = None


def executable(path, marker):
    path.write_text(f"#!/bin/sh\nprintf '%s\\n' '{marker}'\n", encoding="utf-8")
    path.chmod(0o755)


def require(condition, message):
    if not condition:
        raise TestFailure(message)


def command_completion(session, fixture_dir, _workspace, _home):
    executable(fixture_dir / "omni_pty_unique_command", "COMMAND_COMPLETION_OK")
    output = session.submit(b"omni_pty_unique_c\t")
    require(b"COMMAND_COMPLETION_OK" in output, "TAB did not complete and execute the PATH command")


def builtin_completion(session, _fixture_dir, _workspace, home):
    output = session.submit(b"pw\t")
    session.close()
    history = [
        line.rstrip()
        for line in (home / ".omnish_history").read_text(encoding="utf-8").splitlines()
    ]
    require(b"omnish:" not in output, "completed builtin reported an execution error")
    require("pwd" in history, "Readline did not submit the completed builtin name 'pwd'")


def ambiguous_completion(session, fixture_dir, _workspace, _home):
    first = b"omni_pty_amb_alpha"
    second = b"omni_pty_amb_beta"
    executable(fixture_dir / first.decode(), "unused")
    executable(fixture_dir / second.decode(), "unused")
    session.send("ambiguous prefix and TAB", b"omni_pty_amb_\t")
    output = session.read_until_idle()
    require(first in output and second in output, "Readline did not display both ambiguous matches")
    require(output.find(first) < output.find(second), "ambiguous matches were not displayed alphabetically")


def duplicate_suppression(session, fixture_dir, workspace, _home):
    second_dir = workspace / "path-two"
    second_dir.mkdir()
    duplicate = "omni_pty_dup_same"
    executable(fixture_dir / duplicate, "unused")
    executable(second_dir / duplicate, "unused")
    executable(fixture_dir / "omni_pty_dup_sibling", "unused")
    session.send("duplicate prefix and TAB", b"omni_pty_dup_\t")
    output = session.read_until_idle()
    require(output.count(duplicate.encode()) == 1, "duplicate PATH match appeared more than once")


def empty_buffer_tabs(session, fixture_dir, _workspace, _home):
    candidate = b"omni_pty_empty_alpha"
    executable(fixture_dir / candidate.decode(), "unused")
    session.send("first TAB on empty line", b"\t")
    first_output = session.read_until_idle()
    require(b"\a" in first_output, "first empty-line TAB did not ring the terminal bell")
    session.send("second consecutive TAB", b"\t")
    second_output = session.read_until_idle()
    require(candidate in second_output, "second empty-line TAB did not display possible completions")


def completion_query(session, fixture_dir, _workspace, _home):
    last_candidate = b"omni_pty_query_080"
    for number in range(81):
        executable(fixture_dir / f"omni_pty_query_{number:03d}", "unused")
    session.send("first TAB on empty line", b"\t")
    require(b"\a" in session.read_until_idle(), "query test did not receive the first-TAB bell")
    session.send("second TAB requesting all completions", b"\t")
    query = session.read_until(b"possibilities? (y or n)")
    require(b"Display all" in query, "Readline's completion confirmation prompt was not displayed")
    session.send("decline completion list without Enter", b"n")
    cancelled = session.read_until_idle()
    require(last_candidate not in cancelled, "declining the query still displayed the completion list")


def filename_completion(session, _fixture_dir, workspace, _home):
    filename = workspace / "omni_pty_document.txt"
    filename.write_text("FILENAME_COMPLETION_OK\n", encoding="utf-8")
    output = session.submit(b"cat omni_pty_doc\t")
    require(b"FILENAME_COMPLETION_OK" in output, "argument-position filename completion did not reach cat")


def pathname_completion(session, _fixture_dir, workspace, _home):
    executable(workspace / "omni_pty_path_command", "PATHNAME_COMPLETION_OK")
    output = session.submit(b"./omni_pty_path_c\t")
    require(b"PATHNAME_COMPLETION_OK" in output, "slash-containing command did not use pathname completion")


def history_prefix_navigation(session, _fixture_dir, _workspace, _home):
    session.submit(b"printf NAV_ALPHA")
    session.submit(b"printf OTHER_ONE")
    session.submit(b"printf NAV_BETA")
    session.submit(b"printf OTHER_LATEST")
    session.send("history prefix", b"printf NAV_")
    session.send("Up twice, then Down", b"\x1b[A\x1b[A\x1b[B")
    session.send("submit selected history entry", b"\n")
    output = session.read_until(PROMPT_END)
    require(b"NAV_BETA" in output, "Up/Down prefix navigation did not submit the expected matching entry")
    require(b"NAV_ALPHA" not in output, "Down did not move forward from the older prefix match")


CASES = {
    "command-completion": command_completion,
    "builtin-completion": builtin_completion,
    "ambiguous-completion": ambiguous_completion,
    "duplicate-suppression": duplicate_suppression,
    "empty-buffer-tabs": empty_buffer_tabs,
    "completion-query": completion_query,
    "filename-completion": filename_completion,
    "pathname-completion": pathname_completion,
    "history-prefix-navigation": history_prefix_navigation,
}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("case", choices=CASES)
    parser.add_argument("binary")
    parser.add_argument("case_root")
    arguments = parser.parse_args()

    case_root = Path(arguments.case_root)
    workspace = case_root / "workspace"
    home = case_root / "home"
    fixture_dir = case_root / "path-one"
    workspace.mkdir(parents=True)
    home.mkdir()
    fixture_dir.mkdir()

    path_parts = [str(fixture_dir)]
    if arguments.case == "duplicate-suppression":
        path_parts.append(str(workspace / "path-two"))
    if arguments.case in {"filename-completion", "history-prefix-navigation"}:
        path_parts.extend(["/usr/bin", "/bin"])

    session = None
    try:
        session = ShellSession(
            os.path.abspath(arguments.binary), workspace, home, ":".join(path_parts)
        )
        CASES[arguments.case](session, fixture_dir, workspace, home)
        return 0
    except (TestFailure, OSError) as error:
        print(f"PTY case {arguments.case!r} failed: {error}", file=sys.stderr)
        if session is not None:
            print("keys/input sent:", file=sys.stderr)
            for label, data in session.sent:
                print(f"  {label}: {data!r}", file=sys.stderr)
            print("terminal output observed:", file=sys.stderr)
            print(bytes(session.observed).decode("utf-8", errors="replace"), file=sys.stderr)
        return 1
    finally:
        if session is not None:
            session.close()


if __name__ == "__main__":
    sys.exit(main())
