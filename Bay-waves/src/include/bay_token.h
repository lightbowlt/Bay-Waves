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
    TOKEN_RPAREN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_OTHERWISE,
    TOKEN_REPEAT,
    TOKEN_TIMES,
    TOKEN_WHILE,
    TOKEN_FUNCTION,
    TOKEN_WITH,
    TOKEN_RETURN,
    TOKEN_ASK,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_IS,
    TOKEN_EQUAL,
    TOKEN_TO,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_THAN,
    TOKEN_COMMA
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