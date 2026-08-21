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

static AST_T* parser_parse_block(parser_T* parser, int stop_on_otherwise) {
    AST_T* block = ast_new_program();
    if (block == NULL) {
        return NULL;
    }

    while (parser->current != NULL && parser->current->type != TOKEN_EOF) {
        if (parser->current->type == TOKEN_END) {
            break;
        }
        if (stop_on_otherwise && parser->current->type == TOKEN_OTHERWISE) {
            break;
        }
        AST_T* stmt = parser_parse_statement(parser);
        if (stmt != NULL) {
            ast_append_statement(block, stmt);
        }
    }

    return block;
}

static AST_T* parser_parse_function_call(parser_T* parser, const char* name) {
    if (parser == NULL || name == NULL) {
        parser_error(parser, "invalid function call");
        return NULL;
    }

    parser_expect(parser, TOKEN_WITH);

    AST_T** arguments = NULL;
    size_t argument_count = 0;
    while (parser->current != NULL && parser->current->type != TOKEN_DOT && parser->current->type != TOKEN_END) {
        AST_T* argument = parser_parse_comparison(parser);
        arguments = realloc(arguments, (argument_count + 1) * sizeof(AST_T*));
        if (arguments == NULL) {
            parser_error(parser, "memory allocation failed while parsing function call");
        }
        arguments[argument_count++] = argument;
        if (!parser_match(parser, TOKEN_AND)) {
            break;
        }
    }

    AST_T* call = ast_new_function_call(name, arguments, argument_count);
    return call;
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
    if (parser->current->type == TOKEN_ASK) {
        return parser_parse_ask_statement(parser);
    }
    if (parser->current->type == TOKEN_IF) {
        return parser_parse_if_statement(parser);
    }
    if (parser->current->type == TOKEN_REPEAT) {
        return parser_parse_repeat_statement(parser);
    }
    if (parser->current->type == TOKEN_WHILE) {
        return parser_parse_while_statement(parser);
    }
    if (parser->current->type == TOKEN_FUNCTION) {
        return parser_parse_function_declaration(parser);
    }
    if (parser->current->type == TOKEN_RETURN) {
        return parser_parse_return_statement(parser);
    }
    if (parser->current->type == TOKEN_IDENTIFIER) {
        char* name = dup_string(parser->current->value);
        parser_advance(parser);
        if (parser->current != NULL && parser->current->type == TOKEN_WITH) {
            AST_T* call = parser_parse_function_call(parser, name);
            free(name);
            if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
                parser_advance(parser);
            }
            return call;
        }
        if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
            AST_T* call = ast_new_function_call(name, NULL, 0);
            free(name);
            parser_advance(parser);
            return call;
        }
        free(name);
        parser_error(parser, "unexpected identifier in statement");
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

AST_T* parser_parse_ask_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_ASK);
    parser_expect(parser, TOKEN_THE);

    if (parser->current == NULL || parser->current->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "expected variable name after ask the");
    }
    char* name = dup_string(parser->current->value);
    parser_advance(parser);

    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }

    return ast_new_ask_statement(name);
}

AST_T* parser_parse_if_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_IF);
    AST_T* condition = parser_parse_expression(parser);
    parser_expect(parser, TOKEN_THEN);

    AST_T* then_block = parser_parse_block(parser, 1);
    AST_T* else_block = NULL;
    if (parser->current != NULL && parser->current->type == TOKEN_OTHERWISE) {
        parser_advance(parser);
        else_block = parser_parse_block(parser, 0);
    }
    parser_expect(parser, TOKEN_END);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }
    return ast_new_if_statement(condition, then_block, else_block);
}

AST_T* parser_parse_repeat_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_REPEAT);
    AST_T* count_expression = parser_parse_expression(parser);
    parser_expect(parser, TOKEN_TIMES);
    AST_T* body = parser_parse_block(parser, 0);
    parser_expect(parser, TOKEN_END);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }
    return ast_new_repeat_statement(count_expression, body);
}

AST_T* parser_parse_while_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_WHILE);
    AST_T* condition = parser_parse_expression(parser);
    AST_T* body = parser_parse_block(parser, 0);
    parser_expect(parser, TOKEN_END);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }
    return ast_new_while_statement(condition, body);
}

AST_T* parser_parse_function_declaration(parser_T* parser) {
    parser_expect(parser, TOKEN_FUNCTION);
    if (parser->current == NULL || parser->current->type != TOKEN_IDENTIFIER) {
        parser_error(parser, "expected function name");
    }
    char* name = dup_string(parser->current->value);
    parser_advance(parser);

    char** parameter_names = NULL;
    size_t parameter_count = 0;
    if (parser_match(parser, TOKEN_WITH)) {
        if (parser->current != NULL && parser->current->type == TOKEN_IDENTIFIER) {
            do {
                char* param = dup_string(parser->current->value);
                parser_advance(parser);
                parameter_names = realloc(parameter_names, (parameter_count + 1) * sizeof(char*));
                if (parameter_names == NULL) {
                    parser_error(parser, "memory allocation failed while parsing function parameters");
                }
                parameter_names[parameter_count++] = param;
            } while (parser_match(parser, TOKEN_AND));
        }
    }

    AST_T* body = parser_parse_block(parser, 0);
    parser_expect(parser, TOKEN_END);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }

    AST_T* fn = ast_new_function_declaration(name, parameter_names, parameter_count, body);
    free(name);
    return fn;
}

AST_T* parser_parse_return_statement(parser_T* parser) {
    parser_expect(parser, TOKEN_RETURN);
    AST_T* value = parser_parse_expression(parser);
    if (parser->current != NULL && parser->current->type == TOKEN_DOT) {
        parser_advance(parser);
    }
    return ast_new_return_statement(value);
}

AST_T* parser_parse_expression(parser_T* parser) {
    return parser_parse_logical_or(parser);
}

AST_T* parser_parse_logical_or(parser_T* parser) {
    AST_T* left = parser_parse_logical_and(parser);
    while (parser->current != NULL && parser->current->type == TOKEN_OR) {
        parser_advance(parser);
        AST_T* right = parser_parse_logical_and(parser);
        left = ast_new_logical_expression('o', left, right);
    }
    return left;
}

AST_T* parser_parse_logical_and(parser_T* parser) {
    AST_T* left = parser_parse_unary(parser);
    while (parser->current != NULL && parser->current->type == TOKEN_AND) {
        parser_advance(parser);
        AST_T* right = parser_parse_unary(parser);
        left = ast_new_logical_expression('a', left, right);
    }
    return left;
}

AST_T* parser_parse_unary(parser_T* parser) {
    if (parser->current != NULL && parser->current->type == TOKEN_NOT) {
        parser_advance(parser);
        return ast_new_unary_expression('!', parser_parse_unary(parser));
    }
    return parser_parse_comparison(parser);
}

AST_T* parser_parse_comparison(parser_T* parser) {
    AST_T* left = parser_parse_additive(parser);

    if (parser->current != NULL && parser->current->type == TOKEN_IS) {
        parser_advance(parser);
        int negate = 0;
        if (parser_match(parser, TOKEN_NOT)) {
            negate = 1;
        }

        char op[8];
        if (parser_match(parser, TOKEN_EQUAL)) {
            parser_expect(parser, TOKEN_TO);
            snprintf(op, sizeof(op), negate ? "!=" : "==");
            AST_T* right = parser_parse_additive(parser);
            return ast_new_comparison_expression(op, left, right);
        }

        if (parser_match(parser, TOKEN_GREATER)) {
            parser_expect(parser, TOKEN_THAN);
            if (parser_match(parser, TOKEN_OR)) {
                parser_expect(parser, TOKEN_EQUAL);
                parser_expect(parser, TOKEN_TO);
                snprintf(op, sizeof(op), negate ? "<" : ">=");
                AST_T* right = parser_parse_additive(parser);
                return ast_new_comparison_expression(op, left, right);
            }
            snprintf(op, sizeof(op), negate ? "<=" : ">");
            AST_T* right = parser_parse_additive(parser);
            return ast_new_comparison_expression(op, left, right);
        }

        if (parser_match(parser, TOKEN_LESS)) {
            parser_expect(parser, TOKEN_THAN);
            if (parser_match(parser, TOKEN_OR)) {
                parser_expect(parser, TOKEN_EQUAL);
                parser_expect(parser, TOKEN_TO);
                snprintf(op, sizeof(op), negate ? ">" : "<=");
                AST_T* right = parser_parse_additive(parser);
                return ast_new_comparison_expression(op, left, right);
            }
            snprintf(op, sizeof(op), negate ? ">=" : "<");
            AST_T* right = parser_parse_additive(parser);
            return ast_new_comparison_expression(op, left, right);
        }

        parser_error(parser, "invalid comparison expression");
    }

    return left;
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

    if (parser->current->type == TOKEN_TRUE) {
        parser_advance(parser);
        return ast_new_boolean_literal(1);
    }

    if (parser->current->type == TOKEN_FALSE) {
        parser_advance(parser);
        return ast_new_boolean_literal(0);
    }

    if (parser->current->type == TOKEN_IDENTIFIER) {
        char* name = dup_string(parser->current->value);
        parser_advance(parser);
        if (parser->current != NULL && parser->current->type == TOKEN_WITH) {
            AST_T* call = parser_parse_function_call(parser, name);
            free(name);
            return call;
        }
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

