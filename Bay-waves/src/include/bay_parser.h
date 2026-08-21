#ifndef BAY_PARSER_H
#define BAY_PARSER_H

#include "bay_lexer.h"
#include "AST.h"

typedef struct PARSER_STRUCT {
    lexer_T* lexer;
    token_T* current;
} parser_T;

parser_T* init_parser(lexer_T* lexer);
void free_parser(parser_T* parser);
void parser_advance(parser_T* parser);
void parser_expect(parser_T* parser, TokenType type);
int parser_match(parser_T* parser, TokenType type);
AST_T* parser_parse_program(parser_T* parser);
AST_T* parser_parse_statement(parser_T* parser);
AST_T* parser_parse_variable_declaration(parser_T* parser);
AST_T* parser_parse_say_statement(parser_T* parser);
AST_T* parser_parse_ask_statement(parser_T* parser);
AST_T* parser_parse_if_statement(parser_T* parser);
AST_T* parser_parse_repeat_statement(parser_T* parser);
AST_T* parser_parse_while_statement(parser_T* parser);
AST_T* parser_parse_function_declaration(parser_T* parser);
AST_T* parser_parse_return_statement(parser_T* parser);
AST_T* parser_parse_expression(parser_T* parser);
AST_T* parser_parse_logical_or(parser_T* parser);
AST_T* parser_parse_logical_and(parser_T* parser);
AST_T* parser_parse_unary(parser_T* parser);
AST_T* parser_parse_comparison(parser_T* parser);
AST_T* parser_parse_additive(parser_T* parser);
AST_T* parser_parse_multiplicative(parser_T* parser);
AST_T* parser_parse_primary(parser_T* parser);

#endif