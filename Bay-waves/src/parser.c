#include "include/bay_parser.h"
#include <stdio.h>

parser_T* init_parser(lexer_T* lexer){
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    return parser;
}

parser_T* parser_eat(parser_T* parser, int token_type){
    if (parser->current_token->type == token_type) {
        parser->current_token = lexer_get_next_token(parser->lexer);
    } else {
        // Handle error: unexpected token
        printf("Error: Unexpected token type. Expected %d, got %d\n", token_type, parser->current_token->type);
        exit(1); // Exit the program with an error code
    }
    
    return parser;
}

AST_T* parser_parse(parser_T* parser){
    return parser_parse_statements(parser);
}

AST_T* parser_parse_statement(parser_T* parser){}

AST_T* parser_parse_statements(parser_T* parser){

    AST_T* compound = init_ast(AST_COMPOND);

    AST_T** compound_value = calloc(1, sizeof( struct AST_STRUCT*));

    AST_T* ast_statement = parser_parse_statement(parser);

    compound_value[0] = ast_statement;

    while (parser->current_token->type != TOKEN_POINT) {
        compound_value = realloc(compound_value, sizeof(struct AST_STRUCT*) * (ast_statement->compound_size + 1));
        compound_value[ast_statement->compound_size] = ast_statement;
        ast_statement = parser_parse_statement(parser);
    }
}

AST_T* parser_parse_expr(parser_T* parser){}

AST_T* parser_parse_factor(parser_T* parser){}

AST_T* parser_parse_term(parser_T* parser){}

AST_T* parser_parse_function_call(parser_T* parser){}

AST_T* parser_parse_variable(parser_T* parser){}

AST_T* parser_parse_string(parser_T* parser){}

AST_T* parser_parse(parser_T* parser){}
