#include "include/AST.h"
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

AST_T* ast_new_program(void) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_PROGRAM;
    return node;
}

void ast_append_statement(AST_T* program, AST_T* statement) {
    if (program == NULL || statement == NULL) {
        return;
    }
    if (program->statement_count == 0) {
        program->statements = calloc(1, sizeof(AST_T*));
    } else {
        program->statements = realloc(program->statements,
                                     (program->statement_count + 1) * sizeof(AST_T*));
    }
    if (program->statements == NULL) {
        return;
    }
    program->statements[program->statement_count] = statement;
    program->statement_count += 1;
}

AST_T* ast_new_variable_declaration(const char* name, AST_T* value) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_VARIABLE_DECLARATION;
    node->declaration_name = dup_string(name);
    node->declaration_value = value;
    return node;
}

AST_T* ast_new_integer_literal(long value) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_INTEGER_LITERAL;
    node->integer_value = value;
    return node;
}

AST_T* ast_new_string_literal(const char* value) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_STRING_LITERAL;
    node->string_value = dup_string(value);
    return node;
}

AST_T* ast_new_boolean_literal(int value) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_BOOLEAN_LITERAL;
    node->boolean_value = value;
    return node;
}

AST_T* ast_new_binary_expression(char op, AST_T* left, AST_T* right) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_BINARY_EXPRESSION;
    node->binary_operator = op;
    node->left = left;
    node->right = right;
    return node;
}

AST_T* ast_new_unary_expression(char op, AST_T* operand) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_UNARY_EXPRESSION;
    node->unary_operator = op;
    node->operand = operand;
    return node;
}

AST_T* ast_new_comparison_expression(const char* op, AST_T* left, AST_T* right) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_COMPARISON_EXPRESSION;
    node->comparison_operator = dup_string(op == NULL ? "==" : op);
    node->left = left;
    node->right = right;
    return node;
}

AST_T* ast_new_logical_expression(char op, AST_T* left, AST_T* right) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_LOGICAL_EXPRESSION;
    node->logical_operator = op;
    node->left = left;
    node->right = right;
    return node;
}

AST_T* ast_new_variable_reference(const char* name) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_VARIABLE_REFERENCE;
    node->variable_name = dup_string(name);
    return node;
}

AST_T* ast_new_say_statement(AST_T* expression) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_SAY_STATEMENT;
    node->say_expression = expression;
    return node;
}

AST_T* ast_new_ask_statement(const char* name) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_ASK_STATEMENT;
    node->ask_name = dup_string(name);
    return node;
}

AST_T* ast_new_if_statement(AST_T* condition, AST_T* then_block, AST_T* else_block) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_IF_STATEMENT;
    node->condition = condition;
    node->then_block = then_block;
    node->else_block = else_block;
    return node;
}

AST_T* ast_new_repeat_statement(AST_T* count_expression, AST_T* body) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_REPEAT_STATEMENT;
    node->count_expression = count_expression;
    node->body = body;
    return node;
}

AST_T* ast_new_while_statement(AST_T* condition, AST_T* body) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_WHILE_STATEMENT;
    node->condition = condition;
    node->body = body;
    return node;
}

AST_T* ast_new_function_declaration(const char* name, char** parameter_names, size_t parameter_count, AST_T* body) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_FUNCTION_DECLARATION;
    node->function_name = dup_string(name);
    node->parameter_names = parameter_names;
    node->parameter_count = parameter_count;
    node->body = body;
    return node;
}

AST_T* ast_new_function_call(const char* name, AST_T** args, size_t arg_count) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_FUNCTION_CALL;
    node->function_name = dup_string(name);
    node->arguments = args;
    node->argument_count = arg_count;
    return node;
}

AST_T* ast_new_return_statement(AST_T* expression) {
    AST_T* node = calloc(1, sizeof(struct AST_STRUCT));
    if (node == NULL) {
        return NULL;
    }
    node->type = AST_RETURN_STATEMENT;
    node->return_value = expression;
    return node;
}

void ast_free(AST_T* node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->statement_count; ++i) {
                ast_free(node->statements[i]);
            }
            free(node->statements);
            break;
        case AST_VARIABLE_DECLARATION:
            free(node->declaration_name);
            ast_free(node->declaration_value);
            break;
        case AST_INTEGER_LITERAL:
            break;
        case AST_STRING_LITERAL:
            free(node->string_value);
            break;
        case AST_BOOLEAN_LITERAL:
            break;
        case AST_BINARY_EXPRESSION:
            ast_free(node->left);
            ast_free(node->right);
            break;
        case AST_UNARY_EXPRESSION:
            ast_free(node->operand);
            break;
        case AST_COMPARISON_EXPRESSION:
            free(node->comparison_operator);
            ast_free(node->left);
            ast_free(node->right);
            break;
        case AST_LOGICAL_EXPRESSION:
            ast_free(node->left);
            ast_free(node->right);
            break;
        case AST_VARIABLE_REFERENCE:
            free(node->variable_name);
            break;
        case AST_SAY_STATEMENT:
            ast_free(node->say_expression);
            break;
        case AST_ASK_STATEMENT:
            free(node->ask_name);
            break;
        case AST_IF_STATEMENT:
            ast_free(node->condition);
            ast_free(node->then_block);
            ast_free(node->else_block);
            break;
        case AST_REPEAT_STATEMENT:
            ast_free(node->count_expression);
            ast_free(node->body);
            break;
        case AST_WHILE_STATEMENT:
            ast_free(node->condition);
            ast_free(node->body);
            break;
        case AST_FUNCTION_DECLARATION:
            free(node->function_name);
            if (node->parameter_names != NULL) {
                for (size_t i = 0; i < node->parameter_count; ++i) {
                    free(node->parameter_names[i]);
                }
                free(node->parameter_names);
            }
            ast_free(node->body);
            break;
        case AST_FUNCTION_CALL:
            free(node->function_name);
            if (node->arguments != NULL) {
                for (size_t i = 0; i < node->argument_count; ++i) {
                    ast_free(node->arguments[i]);
                }
                free(node->arguments);
            }
            break;
        case AST_RETURN_STATEMENT:
            ast_free(node->return_value);
            break;
        default:
            break;
    }

    free(node);
}

