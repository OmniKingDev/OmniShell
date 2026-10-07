// Holds All Includes For Shell
#include "omnishell.h"

#define OSHTOK_CAPACITY 8

#include "omnicommands.h"
#include "omniparser.h"

OSHToken *osh_tokenizer(const char *line)
{
    // Initialize Local Variables
    OSHToken *token;
    bool in_quotes = false;
    const char *token_cursor = line;
    const char *moving_cursor;

    size_t token_count = 0;
    size_t token_capacity = OSHTOK_CAPACITY;
    if (!(line)) {
        return NULL;
    }

    token = malloc(sizeof(OSHToken) * token_capacity);
    if (!token) {
        fprintf(stderr, OSH_ERROR"omnish: malloc failed to allocate memory\n"OSH_RESET);
        return NULL;
    }

    while (*token_cursor != '\0') {
        // Skip Whitespace Characters
        if (isspace((unsigned char)*token_cursor) != 0) {
            token_cursor++;
            continue;
        }
        // Ensure Space For Another OSHToken
        if ((token_count + 1) >= token_capacity) {
            token_capacity *= 2;
            OSHToken *new_token = realloc(token, sizeof(OSHToken) * token_capacity);
            if (!new_token) {
                osh_free_tokens(token);
            }
            token = new_token;
        }

        // Copy Current OSHToken Pointer To Moving Pointer
        moving_cursor = token_cursor;
        // Create An OSHToken Operator If Special Operator Is Found
        if (*moving_cursor == '|') {
            token[token_count] = (OSHToken) {
                .token.type = TOKEN_PIPE,
                .token.value = NULL
            };
            token_count++;
            token_cursor++;
            continue;
        } else if (*moving_cursor == '<') {
            token[token_count] = (OSHToken) {
                .token.type = TOKEN_REDIRECT_IN,
                .token.value = NULL
            };
            token_count++;
            token_cursor++;
            continue;
        } else if (*moving_cursor == '>') {
            if (*(moving_cursor + 1) == '>') {
                token[token_count] = (OSHToken) {
                    .token.type = TOKEN_APPEND_OUT,
                    .token.value = NULL
                };
                token_cursor += 2;
            } else {
                token[token_count] = (OSHToken) {
                    .token.type = TOKEN_REDIRECT_OUT,
                    .token.value = NULL
                };
                token_cursor++;
            }
            token_count++;
            continue;
        }

        // Otherwise Start Reading A Word
        in_quotes = false;
        while (*moving_cursor != '\0') {
            if (*moving_cursor == '\"' || *moving_cursor == '\'') {
                in_quotes = !in_quotes;
                moving_cursor++;
                continue;
            }

            // Operators And Whitespace Only End Word Outside Quotes
            if (!in_quotes
                && (isspace((unsigned char)*moving_cursor) != 0
                    || *moving_cursor == '|'
                    || *moving_cursor == '<'
                    || *moving_cursor == '>')) {
                break;
            }
            moving_cursor++;
        }

        // Opening Quote Was Never Closed
        if (in_quotes) {
            fprintf(stderr, OSH_ERROR "osh: missing an ending '\"' or '\''\n" OSH_RESET);
            token[token_count] = (OSHToken) {
                .token.type = TOKEN_EOF,
                .token.value = NULL
            };
            osh_free_tokens(token);
            return NULL;
        }

        /*
         * NOTE:
         *  Grab address and subtract where the address of token cursor is
         *  which then can give you the exact starting index for given token.
         *  Same idea is applied to grab index at the end of token(excluding the '\0').
         */
        token[token_count].index.start_index = (size_t)(token_cursor - line);
        size_t word_len = moving_cursor - token_cursor;
        token[token_count].index.end_index = (size_t)(moving_cursor - line - 1);
        token[token_count].token.value = malloc(word_len + 1);
        if(!token[token_count].token.value) {
            osh_free_tokens(token);
            return NULL;
        }
        token[token_count].token.type = TOKEN_WORD;

        // Copy Word While Removing Quote Characters
        size_t value_index = 0;
        for (const char *pos = token_cursor; pos < moving_cursor; pos++) {
            if (*pos != '\"' && *pos != '\'') {
                token[token_count].token.value[value_index] = *pos;
                value_index++;
            }
        }
        token[token_count].token.value[value_index] = '\0';
        token_count++;
        // Next OSHToken Begins Where Moving Cursor Stopped
        token_cursor = moving_cursor;
    }

    // Mark End Of OSHToken Array
    token[token_count] = (OSHToken) {
        .token.type = TOKEN_EOF,
        .token.value = NULL
    };
    osh_organize_tokens(token);
    return token;
}

void osh_organize_tokens(OSHToken *token)
{
    /* Check For Word Type, IF Command/Arg(s)/Directories OR User Given Name(UNKNOWN) */
    for (size_t i = 0; token[i].token.type != TOKEN_EOF; i++) {
        switch (token[i].token.type) {
            case TOKEN_WORD:
                if (token[i].token.value == NULL) {
                    break;
                }
                // 'osh_is_builtin' Returns 1 If True, Is A Builtin
                if (osh_is_builtin(token[i].token.value)) {
                    token[i].string = TEXT_BUILTIN;
                    token[i].usage = ROLE_COMMAND;
                    break;
                }
                char *env_path;
                env_path = osh_find_executable(token[i].token.value);
                if (env_path != NULL) {
                    token[i].string = TEXT_EXEC;
                    token[i].usage = ROLE_COMMAND;
                    free(env_path);
                    break;
                }
                // printf("[%zu] WORD: %s\n", i, token[i].token.value);
                token[i].string = TEXT_UNKNOWN;
                token[i].usage = ROLE_ARG;
                break;
            case TOKEN_PIPE:
                token[i].usage = ROLE_PIPING;
               //printf("[%zu] PIPE\n", i);
                break;
            case TOKEN_REDIRECT_IN:
                token[i].usage = ROLE_REDIRECT_IN;
               //printf("[%zu] REDIRECT_IN\n", i);
                break;
            case TOKEN_REDIRECT_OUT:
                token[i].usage = ROLE_OWRITE_OCREATE;
               //printf("[%zu] REDIRECT_OUT\n", i);
                break;
            case TOKEN_APPEND_OUT:
                token[i].usage = ROLE_APPEND;
               //printf("[%zu] APPEND_OUT\n", i);
                break;
            default:
                break;
        }
    }
}

void osh_free_tokens(OSHToken *token)
{
    if (!token) {
        return;
    }

    for (size_t i = 0; token[i].token.type != TOKEN_EOF; i++) {

        free(token[i].token.value);
    }

    free(token);
}
