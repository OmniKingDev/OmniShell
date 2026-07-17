# OmniShell Project Instructions

## Project goal

OmniShell is Demetrius Jackson's CS50 final project and an important
portfolio project for entering professional software engineering.

The immediate priority is to finish a reliable, understandable shell,
submit it for the CS50 certificate, and package it professionally.

## Collaboration rules

- Preserve Demetrius's naming, intent, comments, and control flow.
- Do not redesign unrelated parts of the project.
- Make the smallest change that establishes a clean working foundation.
- Explain unfamiliar C syntax before introducing it.
- Do not remove working behavior without approval.
- Ask before running commands or making broad changes.
- Show diffs clearly.
- Keep the code understandable to its author.
- Correct actual compiler, memory, linkage, and control-flow problems.
- Prioritize a finished product over premature complexity.

## Current architecture target

- omnishell.c: program entry point only
- omnishell.h: public shell-loop declaration
- omnifunc.c: shell loop, input, parsing, execution, and process launching
- omnifunc.h: public declarations for omnifunc.c
- omnibuiltins.c: built-in tables, dispatch logic, and implementations
- omnibuiltins.h: public declarations for the built-in subsystem
- Makefile: builds all translation units

Do not include .c files inside other .c or .h files.

Headers should primarily contain declarations, macros, and shared types.
Function implementations should live in .c files.

## Build expectations

Compile using strict warnings:

gcc -std=c17 -Wall -Wextra -Wpedantic

Do not suppress warnings merely to make the build appear successful.
