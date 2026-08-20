#include "include/bay_lexer.h"
#include "include/bay_token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* dup_string(const char* value) {
    size_t len = value == NULL ? 0 : strlen(value);
    char* copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    if (len > 0) {
        memcpy(copy, value, len);
    }
    copy[len] = '\0';
    return copy;
}

static void lexer_error(lexer_T* lexer, const char* message) {
    fprintf(stderr, "error: %s at line %d, column %d\n", message, lexer->line, lexer->column);
    exit(1);
}

static void append_char(char** buffer, size_t* length, size_t* capacity, char c) {
    if (*length + 1 >= *capacity) {
        *capacity = *capacity == 0 ? 16 : *capacity * 2;
        char* next = realloc(*buffer, *capacity);
        if (next == NULL) {
            free(*buffer);
            exit(1);
        }
        *buffer = next;
    }
    (*buffer)[*length] = c;
    *length += 1;
    (*buffer)[*length] = '\0';
}

lexer_T* init_lexer(const char* contents) {
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
    if (lexer == NULL) {
        return NULL;
    }

    lexer->contents = dup_string(contents == NULL ? "" : contents);
    lexer->length = strlen(lexer->contents);
    lexer->index = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current = lexer->contents[0];
    return lexer;
}

void free_lexer(lexer_T* lexer) {
    if (lexer == NULL) {
        return;
    }
    free(lexer->contents);
    free(lexer);
}

void lexer_advance(lexer_T* lexer) {
    if (lexer == NULL || lexer->current == '\0') {
        return;
    }

    if (lexer->current == '\n') {
        lexer->line += 1;
        lexer->column = 1;
    } else {
        lexer->column += 1;
    }

    lexer->index += 1;
    if (lexer->index >= lexer->length) {
        lexer->current = '\0';
    } else {
        lexer->current = lexer->contents[lexer->index];
    }
}

void lexer_skip_whitespace(lexer_T* lexer) {
    while (lexer->current != '\0' &&
           (lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\t' || lexer->current == '\r')) {
        lexer_advance(lexer);
    }
}

token_T* lexer_collect_number(lexer_T* lexer) {
    int start_line = lexer->line;
    int start_col = lexer->column;
    char* value = calloc(1, sizeof(char));
    size_t length = 0;
    size_t capacity = 1;

    while (lexer->current != '\0' && isdigit((unsigned char)lexer->current)) {
        append_char(&value, &length, &capacity, lexer->current);
        lexer_advance(lexer);
    }

    token_T* token = init_token(TOKEN_INTEGER, value, start_line, start_col);
    free(value);
    return token;
}

token_T* lexer_collect_identifier(lexer_T* lexer) {
    int start_line = lexer->line;
    int start_col = lexer->column;
    char* value = calloc(1, sizeof(char));
    size_t length = 0;
    size_t capacity = 1;

    while (lexer->current != '\0' && (isalnum((unsigned char)lexer->current) || lexer->current == '_')) {
        append_char(&value, &length, &capacity, lexer->current);
        lexer_advance(lexer);
    }

    TokenType type = TOKEN_IDENTIFIER;
    if (strcmp(value, "variable") == 0) type = TOKEN_VARIABLE;
    else if (strcmp(value, "equals") == 0) type = TOKEN_EQUALS;
    else if (strcmp(value, "say") == 0) type = TOKEN_SAY;
    else if (strcmp(value, "the") == 0) type = TOKEN_THE;
    else if (strcmp(value, "end") == 0) type = TOKEN_END;

    token_T* token = init_token(type, value, start_line, start_col);
    free(value);
    return token;
}

token_T* lexer_collect_string(lexer_T* lexer) {
    int start_line = lexer->line;
    int start_col = lexer->column;
    char* value = calloc(1, sizeof(char));
    size_t length = 0;
    size_t capacity = 1;

    lexer_advance(lexer);
    while (lexer->current != '\0') {
        if (lexer->current == '"') {
            lexer_advance(lexer);
            break;
        }

        if (lexer->current == '\\') {
            char next = lexer->contents[lexer->index + 1];
            if (next == '\0') {
                lexer_error(lexer, "unterminated string literal");
            }
            if (next == 'n') {
                append_char(&value, &length, &capacity, '\n');
                lexer_advance(lexer);
                lexer_advance(lexer);
                continue;
            } else if (next == 't') {
                append_char(&value, &length, &capacity, '\t');
                lexer_advance(lexer);
                lexer_advance(lexer);
                continue;
            } else if (next == '\\') {
                append_char(&value, &length, &capacity, '\\');
                lexer_advance(lexer);
                lexer_advance(lexer);
                continue;
            } else if (next == '"') {
                append_char(&value, &length, &capacity, '"');
                lexer_advance(lexer);
                lexer_advance(lexer);
                continue;
            } else if (isalpha((unsigned char)next) || next == '_') {
                append_char(&value, &length, &capacity, '\\');
                lexer_advance(lexer);
                continue;
            } else {
                char message[128];
                snprintf(message, sizeof(message), "unsupported escape sequence \\%c", next);
                lexer_error(lexer, message);
            }
        }

        append_char(&value, &length, &capacity, lexer->current);
        lexer_advance(lexer);
    }

    if (lexer->current == '\0' && value != NULL && (length == 0 || value[length - 1] != '"')) {
        free(value);
        lexer_error(lexer, "unterminated string literal");
    }

    token_T* token = init_token(TOKEN_STRING, value, start_line, start_col);
    free(value);
    return token;
}

token_T* lexer_get_next_token(lexer_T* lexer) {
    if (lexer == NULL) {
        return NULL;
    }

    while (lexer->current != '\0') {
        if (lexer->current == '#' ) {
            while (lexer->current != '\0' && lexer->current != '\n') {
                lexer_advance(lexer);
            }
            continue;
        }

        if (lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\t' || lexer->current == '\r') {
            lexer_skip_whitespace(lexer);
            continue;
        }

        if (isdigit((unsigned char)lexer->current)) {
            return lexer_collect_number(lexer);
        }

        if (isalpha((unsigned char)lexer->current) || lexer->current == '_') {
            return lexer_collect_identifier(lexer);
        }

        if (lexer->current == '"') {
            return lexer_collect_string(lexer);
        }

        int start_line = lexer->line;
        int start_col = lexer->column;
        char current = lexer->current;
        lexer_advance(lexer);

        switch (current) {
            case '=': return init_token(TOKEN_EQUALS, "=", start_line, start_col);
            case '+': return init_token(TOKEN_PLUS, "+", start_line, start_col);
            case '-': return init_token(TOKEN_MINUS, "-", start_line, start_col);
            case '*': return init_token(TOKEN_MULTIPLY, "*", start_line, start_col);
            case '/': return init_token(TOKEN_DIVIDE, "/", start_line, start_col);
            case '.': return init_token(TOKEN_DOT, ".", start_line, start_col);
            case '(': return init_token(TOKEN_LPAREN, "(", start_line, start_col);
            case ')': return init_token(TOKEN_RPAREN, ")", start_line, start_col);
            default:
                char message[128];
                snprintf(message, sizeof(message), "unexpected character '%c'", current);
                lexer_error(lexer, message);
                return NULL;
        }
    }

    return init_token(TOKEN_EOF, "", lexer->line, lexer->column);
}

char* lexer_get_current_char_as_string(lexer_T* lexer) {
    char* str = calloc(2, sizeof(char));
    if (str == NULL) {
        return NULL;
    }
    str[0] = lexer->current;
    str[1] = '\0';
    return str;
}
