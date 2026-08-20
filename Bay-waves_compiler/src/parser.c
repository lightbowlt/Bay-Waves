#include "include/bay_parser.h"
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

static void parser_error(parser_T* parser, const char* message) {
    if (parser == NULL || parser->current == NULL) {
        fprintf(stderr, "error: %s\n", message);
        exit(1);
    }
    fprintf(stderr, "error: %s at line %d, column %d\n",
            message,
            parser->current->line,
            parser->current->column);
    exit(1);
}

parser_T* init_parser(lexer_T* lexer) {
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    if (parser == NULL) {
        return NULL;
    }
    parser->lexer = lexer;
    parser->current = lexer_get_next_token(lexer);
    return parser;
}

void free_parser(parser_T* parser) {
    if (parser == NULL) {
        return;
    }
    if (parser->current != NULL) {
        free_token(parser->current);
        parser->current = NULL;
    }
    free(parser);
}

void parser_advance(parser_T* parser) {
    if (parser == NULL || parser->current == NULL) {
        return;
    }
    free_token(parser->current);
    parser->current = lexer_get_next_token(parser->lexer);
}

void parser_expect(parser_T* parser, TokenType type) {
    if (parser == NULL || parser->current == NULL) {
        parser_error(parser, "unexpected end of input");
    }
    if (parser->current->type != type) {
        char message[128];
        snprintf(message, sizeof(message), "expected token %d but found %d", type, parser->current->type);
        parser_error(parser, message);
    }
    parser_advance(parser);
}

int parser_match(parser_T* parser, TokenType type) {
    if (parser == NULL || parser->current == NULL) {
        return 0;
    }
    if (parser->current->type == type) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

AST_T* parser_parse_program(parser_T* parser) {
    AST_T* program = ast_new_program();
    if (program == NULL) {
        return NULL;
    }

    while (parser->current != NULL && parser->current->type != TOKEN_EOF) {
        AST_T* stmt = parser_parse_statement(parser);
        if (stmt != NULL) {
            ast_append_statement(program, stmt);
        }
    }

    return program;
}

AST_T* parser_parse_statement(parser_T* parser) {
    if (parser == NULL || parser->current == NULL) {
        return NULL;
    }

    if (parser->current->type == TOKEN_VARIABLE) {
        return parser_parse_variable_declaration(parser);
    }
    if (parser->current->type == TOKEN_SAY) {
        return parser_parse_say_statement(parser);
    }

    char message[128];
    snprintf(message, sizeof(message), "unexpected token type %d", parser->current->type);
    parser_error(parser, message);
    return NULL;
}

AST_T* parser_parse_variable_declaration(parser_T* parser) {
    parser_expect(parser, TOKEN_VARIABLE);

    if (parser->current == NULL || parser->current->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "expected variable name");
    }
    char* name = dup_string(parser->current->value);
    parser_advance(parser);

    parser_expect(parser, TOKEN_EQUALS);
    AST_T* value = parser_parse_expression(parser);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }

    AST_T* declaration = ast_new_variable_declaration(name, value);
    free(name);
    return declaration;
}

AST_T* parser_parse_say_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_SAY);

    AST_T* expression = NULL;
    if (parser_match(parser, TOKEN_THE)) {
        expression = parser_parse_expression(parser);
        parser_expect(parser, TOKEN_END);
    } else {
        expression = parser_parse_expression(parser);
    }

    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }

    return ast_new_say_statement(expression);
}

AST_T* parser_parse_expression(parser_T* parser) {
    return parser_parse_additive(parser);
}

AST_T* parser_parse_additive(parser_T* parser) {
    AST_T* left = parser_parse_multiplicative(parser);

    while (parser->current != NULL &&
           (parser->current->type == TOKEN_PLUS || parser->current->type == TOKEN_MINUS)) {
        char op = parser->current->value[0];
        parser_advance(parser);
        AST_T* right = parser_parse_multiplicative(parser);
        left = ast_new_binary_expression(op, left, right);
    }

    return left;
}

AST_T* parser_parse_multiplicative(parser_T* parser) {
    AST_T* left = parser_parse_primary(parser);

    while (parser->current != NULL &&
           (parser->current->type == TOKEN_MULTIPLY || parser->current->type == TOKEN_DIVIDE)) {
        char op = parser->current->value[0];
        parser_advance(parser);
        AST_T* right = parser_parse_primary(parser);
        left = ast_new_binary_expression(op, left, right);
    }

    return left;
}

AST_T* parser_parse_primary(parser_T* parser) {
    if (parser == NULL || parser->current == NULL) {
        parser_error(parser, "unexpected end of input in expression");
    }

    if (parser->current->type == TOKEN_INTEGER) {
        char* end = NULL;
        long value = strtol(parser->current->value, &end, 10);
        parser_advance(parser);
        return ast_new_integer_literal(value);
    }

    if (parser->current->type == TOKEN_STRING) {
        char* value = dup_string(parser->current->value);
        parser_advance(parser);
        AST_T* literal = ast_new_string_literal(value);
        free(value);
        return literal;
    }

    if (parser->current->type == TOKEN_IDENTIFIER) {
        char* name = dup_string(parser->current->value);
        parser_advance(parser);
        AST_T* reference = ast_new_variable_reference(name);
        free(name);
        return reference;
    }

    if (parser_match(parser, TOKEN_LPAREN)) {
        AST_T* expression = parser_parse_expression(parser);
        parser_expect(parser, TOKEN_RPAREN);
        return expression;
    }

    parser_error(parser, "invalid expression");
    return NULL;
}

