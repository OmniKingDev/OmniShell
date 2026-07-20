                    --- 12 JULY, 2026. Demetrius Jackson A.K.A OmniKing ---
                            LEARNING FROM ALREADY BUILT SHELL
    """ Workout Small Scripts """
  1. Create a folder to practice writing some C and C++ code, any small programs related to shell building.
  2. Learn the foundation of what a shell is and the blueprint needed to start.
  3. Generate 3 or more files that display the blueprint of the starting functions and priorities of 'OmniShell'.
  4. Learn how to create Header files (example.h).


                          --- 13 JULY, 2026. OmniKing ---
                            BLUEPRINT FOR OMNISHELL
    """ Blueprint for OMNIShell """
• [Phase 1: Basic MVP] ───> [Phase 2: Core Shell Features] ───> [Phase 3: Portfolio Polish]
• Infinite Read-Eval Loop  • Built-in commands (cd, exit)     • Piping (|)
• Tokenize input spaces    • Environment variables            • Redirection (<, >)
• Fork & exec processes    • Signal handling (Ctrl+C)         • Background runs (&)

  1. Shell version 1:
      Focus on building REPL (Read Evaluate Print/Execute Loop)
      ----------------------------------------------------------------
        - Start By Creating A Infinite Loop that only waits for a exit or CTRL-C command.


                          --- 15 JULY, 2026. OmniKing ---
                            WRITING OMNISHELL STARTING FILES
    """ Building Files and Starting Shell"""

  1. Continuing From 13 July:
      Building starting scripts (omnishell.c, omnishell.h, omnifunc.h)
      ----------------------------------------------------------------
        - omnishell.c
          -> Main shell file creating print banner and loop
          -> Only job is to present/start shell function
        - omnishell.h
          -> Holds all header files and omnifunc.h header file
          -> Create omnish function in omnishell.h header file
          -> Include any other header files that the omnish function needs
        - omnifunc.h
          -> Create any functions the omnish function relies only
          -> Anything that could be stretch to other future programs


                          --- 17 JULY, 2026. OmniKing ---
                            OMNISHELL FOUNDATION FILES
    """ Structure Shell Files """
  Using Codex to Re-structure Shell Files

  1. Continuing From 15 July:
      Building more scripts (omnifunc.c, omnibuiltins.h, omnibuiltins.c)
      ----------------------------------------------------------------
        - omnifunc.c
          -> Holds Shell Loop, Input Reading, Token Parsing, Command Execution, External-Program Launching, Current-Directory Retrieval, And Prompt-Path Formatting
        - omnibuiltins.c
          -> Holds Private Builtin Command/Function Tables, Builtin Dispatch, And Completed Builtin Implementations Such As cd, help, exit, And pwd
        - omnibuiltins.h
          -> Declares Public Builtin Functions Used By The Rest Of OmniShell


                          --- 18 JULY, 2026. OmniKing ---
                            RESEARCHING NEXT OMNISHELL BUILTIN
    """ Plan Today's OmniShell Work """

  1. Research additional builtin commands normally expected from a shell.
  2. Choose and begin the next OmniShell builtin.
  3. Continue testing pwd and the shortened current-directory prompt.
  4. Plan the explicit omnirun <file> builtin for Python, C, and C++.
  5. Keep automatic file recognition as later work after omnirun is reliable.
