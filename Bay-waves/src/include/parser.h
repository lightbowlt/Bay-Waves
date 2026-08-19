#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "AST.h"

typedef struct PARSER_STRUCT {
    lexer_T* lexer;
    token_T* current_token;
} parser_T;

parser_T* init_parser(lexer_T* lexer);

parser_T* parser_eat(parser_T* parser, int token_type);

AST_T* parser_parse(parser_T* parser);

#endif