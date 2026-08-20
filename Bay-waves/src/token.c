#include "include/bay_token.h"
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

token_T* init_token(TokenType type, const char* value, int line, int column) {
    token_T* token = calloc(1, sizeof(struct TOKEN_STRUCT));
    if (token == NULL) {
        return NULL;
    }

    token->type = type;
    token->value = dup_string(value);
    token->line = line;
    token->column = column;

    return token;
}

void free_token(token_T* token) {
    if (token == NULL) {
        return;
    }
    free(token->value);
    free(token);
}