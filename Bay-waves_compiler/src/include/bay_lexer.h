#ifndef BAY_LEXER_H
#define BAY_LEXER_H

#include <stddef.h>
#include "bay_token.h"

typedef struct LEXER_STRUCT {
    char* contents;
    size_t length;
    size_t index;
    char current;
    int line;
    int column;
} lexer_T;

lexer_T* init_lexer(const char* contents);
void free_lexer(lexer_T* lexer);
void lexer_advance(lexer_T* lexer);
void lexer_skip_whitespace(lexer_T* lexer);
token_T* lexer_get_next_token(lexer_T* lexer);
token_T* lexer_collect_string(lexer_T* lexer);
token_T* lexer_collect_identifier(lexer_T* lexer);
token_T* lexer_collect_number(lexer_T* lexer);
char* lexer_get_current_char_as_string(lexer_T* lexer);

#endif