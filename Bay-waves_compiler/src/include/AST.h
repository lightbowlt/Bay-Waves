#ifndef BAY_AST_H
#define BAY_AST_H

#include <stddef.h>

typedef struct AST_STRUCT AST_T;

enum {
    AST_PROGRAM,
    AST_VARIABLE_DECLARATION,
    AST_INTEGER_LITERAL,
    AST_STRING_LITERAL,
    AST_BINARY_EXPRESSION,
    AST_VARIABLE_REFERENCE,
    AST_SAY_STATEMENT
};

struct AST_STRUCT {
    int type;

    /* program */
    AST_T** statements;
    size_t statement_count;

    /* variable declaration */
    char* declaration_name;
    AST_T* declaration_value;

    /* integer literal */
    long integer_value;

    /* string literal */
    char* string_value;

    /* binary expression */
    char binary_operator;
    AST_T* left;
    AST_T* right;

    /* variable reference */
    char* variable_name;

    /* say statement */
    AST_T* say_expression;
};

AST_T* ast_new_program(void);
void ast_append_statement(AST_T* program, AST_T* statement);
AST_T* ast_new_variable_declaration(const char* name, AST_T* value);
AST_T* ast_new_integer_literal(long value);
AST_T* ast_new_string_literal(const char* value);
AST_T* ast_new_binary_expression(char op, AST_T* left, AST_T* right);
AST_T* ast_new_variable_reference(const char* name);
AST_T* ast_new_say_statement(AST_T* expression);
void ast_free(AST_T* node);

#endif