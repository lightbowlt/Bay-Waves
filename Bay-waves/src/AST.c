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
        case AST_BINARY_EXPRESSION:
            ast_free(node->left);
            ast_free(node->right);
            break;
        case AST_VARIABLE_REFERENCE:
            free(node->variable_name);
            break;
        case AST_SAY_STATEMENT:
            ast_free(node->say_expression);
            break;
        default:
            break;
    }

    free(node);
}

