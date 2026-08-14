
## To Complete Lines Of Commands Then Files/Directories

FUNCTION initialize_readline
    initialize readline
    tell readline to use OmniShell completion function
END FUNCTION


FUNCTION complete_command(text, start, end)
    IF start is not 0
        return NULL
        // let readline do normal file completion
    END IF

    ask readline to build matches
    use OmniShell builtin generator
    return matches
END FUNCTION


FUNCTION builtin_generator(text, state)
    IF this is first call
        reset builtin index to 0
        get length of text
    END IF

    WHILE there are more builtins
        get builtin at current index
        move index forward

        IF beginning of builtin matches text
            return copy of builtin
        END IF
    END WHILE

    return NULL
END FUNCTION


## Entering The Builtins

FUNCTION get_builtin(index)
    IF index is invalid
        return NULL
    END IF

    return builtin_command[index]
END FUNCTION
