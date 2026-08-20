#ifndef BAY_TOKEN_H
#define BAY_TOKEN_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_VARIABLE,
    TOKEN_EQUALS,
    TOKEN_SAY,
    TOKEN_THE,
    TOKEN_END,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_DOT,
    TOKEN_LPAREN,
    TOKEN_RPAREN
} TokenType;

typedef struct TOKEN_STRUCT {
    TokenType type;
    char* value;
    int line;
    int column;
} token_T;

token_T* init_token(TokenType type, const char* value, int line, int column);
void free_token(token_T* token);

#endif