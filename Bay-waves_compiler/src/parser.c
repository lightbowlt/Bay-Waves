#include "include/bay_parser.h"
#include <stdio.h>
#include <string.h>

parser_T* init_parser(lexer_T* lexer){
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);
    return parser;
}

void parser_eat(parser_T* parser, int token_type){
    if (parser->current_token->type == token_type) {
        parser->current_token = lexer_get_next_token(parser->lexer);
    } else {
        // Handle error: unexpected token
        printf("Error: Unexpected token type. Expected %d, got %d\n", token_type, parser->current_token->type);
        exit(1); // Exit the program with an error code
    }
}

AST_T* parser_parse(parser_T* parser)
{
    return parser_parse_statements(parser);
}

AST_T* parser_parse_statement(parser_T* parser){
    switch (parser->current_token->type)
    {
    case TOKEN_ID: return parser_parse_id(parser);
        /* code */
        break;
    
    default:
        break;
    }
}

AST_T* parser_parse_statements(parser_T* parser){

    AST_T* compound = init_ast(AST_COMPOND);

    compound->compound_value = calloc(1, sizeof( struct AST_STRUCT*));

    AST_T* ast_statement = parser_parse_statement(parser);
    compound->compound_value[0] = ast_statement;
    compound->compound_size += 1;

    while (parser->current_token->type != TOKEN_POINT) {
        parser_eat(parser, TOKEN_POINT);
        AST_T* ast_statement = parser_parse_statement(parser);
        compound->compound_size += 1;
        compound->compound_value = realloc(compound->compound_value, compound->compound_size * sizeof(struct AST_STRUCT*));
        compound->compound_value[compound->compound_size-1] = ast_statement;
        
    }
    return compound;
}

AST_T* parser_parse_expr(parser_T* parser){
    return parser_parse_term(parser);
}

AST_T* parser_parse_factor(parser_T* parser){
    if (parser->current_token->type == TOKEN_STRING) {
        AST_T* string_node = init_ast(AST_STRING);
        string_node->string_value = parser->current_token->value;
        parser_eat(parser, TOKEN_STRING);
        return string_node;
    }
    
    if (parser->current_token->type == TOKEN_ID) {
        return parser_parse_variable(parser);
    }
    
    return (void*)0;
}

AST_T* parser_parse_term(parser_T* parser){
    return parser_parse_factor(parser);
}

AST_T* parser_parse_function_call(parser_T* parser){}

AST_T* parser_parse_variable_definition(parser_T* parser){
    parser_eat(parser, TOKEN_ID);
    char* variable_definition_variable_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_EQUALS);
    AST_T* variable_defenition_variable_value = parser_parse_expr(parser);

    AST_T* variable_definition = init_ast(AST_VARIABLE_DEFENITION);
    variable_definition->variable_defenition_variable_name = variable_definition_variable_name;
    variable_definition->variable_defenition_variable_value = variable_defenition_variable_value;
    return variable_definition;
}

AST_T* parser_parse_variable(parser_T* parser){
    parser_eat(parser, TOKEN_ID);
    if (parser->current_token->type == TOKEN_LPAREN) {
        return parser_parse_function_call(parser);
    } 
        AST_T* variable = init_ast(AST_VARIABLE);
        variable->variable_name = parser->prev_token->value;
        return variable;
}

AST_T* parser_parse_string(parser_T* parser){}

AST_T* parser_parse_id(parser_T* parser){
    if (strcmp(parser->current_token->value, "variable") == 0) {
        return parser_parse_variable_definition(parser);
    }
    else {
        return parser_parse_variable(parser);
    }
}
