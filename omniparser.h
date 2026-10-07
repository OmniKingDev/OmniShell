#pragma once

#ifndef OMNISHELL_H
    #include "omnishell.h"
#endif

/* The Core Data Structures */
// TokenType Enum
typedef enum {
    TOKEN_WORD,         // Standard command or argument (e.g., "ls", "-la")
    TOKEN_PIPE,         // |
    TOKEN_REDIRECT_IN,  // <
    TOKEN_REDIRECT_OUT, // >
    TOKEN_APPEND_OUT,   // >>
    TOKEN_EOF,          // Mark end of token array
} TokenType;

typedef struct {
    TokenType type;
    char *value;    // The actual test slice allocated dynamically
} TokenTextType;

typedef enum {
    TEXT_UNKNOWN, // For Arbatrary Words
    TEXT_FILE,    // Is a Regular File
    TEXT_EXEC,    // Is an executable
    TEXT_DIR,     // Is a Directory
    TEXT_BUILTIN, // Is a Osh Built-In
} TextType;

typedef enum {
    ROLE_ARG,            // Argument For Commands/Built-Ins
    ROLE_COMMAND,        // Commands/Built-Ins
    ROLE_PIPING,         // Pipe To Commands Together
    ROLE_REDIRECT_IN,    // Redircting System-File-Descriptors Input
    ROLE_OWRITE_OCREATE, // Create/Overwrite Files
    ROLE_APPEND,         // Append To file
} TokenRole;

typedef struct {
    size_t start_index;  // Start Index For Line Logic
    size_t end_index;    // Ending Index For Line Logic
} TokenLocation;

typedef struct {
    TokenLocation index; // Where Token Start/End In Line
    TokenTextType token; // Syntax Checking
    TextType string;     // Indentity Of Token/Arguments
    TokenRole usage;     // Role For Execution
} OSHToken;

OSHToken *osh_tokenizer(const char *line);
void osh_free_tokens(OSHToken *token);
void osh_organize_tokens(OSHToken *token);
