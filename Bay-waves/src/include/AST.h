#ifndef BAY_AST_H
#define BAY_AST_H

#include <stddef.h>

typedef struct AST_STRUCT AST_T;

enum {
    AST_PROGRAM,
    AST_VARIABLE_DECLARATION,
    AST_INTEGER_LITERAL,
    AST_STRING_LITERAL,
    AST_BOOLEAN_LITERAL,
    AST_BINARY_EXPRESSION,
    AST_UNARY_EXPRESSION,
    AST_COMPARISON_EXPRESSION,
    AST_LOGICAL_EXPRESSION,
    AST_VARIABLE_REFERENCE,
    AST_SAY_STATEMENT,
    AST_ASK_STATEMENT,
    AST_IF_STATEMENT,
    AST_REPEAT_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_CALL,
    AST_RETURN_STATEMENT
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

    /* boolean literal */
    int boolean_value;

    /* expression nodes */
    char binary_operator;
    char unary_operator;
    char logical_operator;
    char* comparison_operator;
    AST_T* left;
    AST_T* right;
    AST_T* operand;

    /* variable reference */
    char* variable_name;

    /* say statement */
    AST_T* say_expression;

    /* ask statement */
    char* ask_name;

    /* control flow */
    AST_T* condition;
    AST_T* then_block;
    AST_T* else_block;
    AST_T* count_expression;
    AST_T* body;

    /* function declarations */
    char* function_name;
    char** parameter_names;
    size_t parameter_count;
    AST_T** arguments;
    size_t argument_count;

    /* return */
    AST_T* return_value;
};

AST_T* ast_new_program(void);
void ast_append_statement(AST_T* program, AST_T* statement);
AST_T* ast_new_variable_declaration(const char* name, AST_T* value);
AST_T* ast_new_integer_literal(long value);
AST_T* ast_new_string_literal(const char* value);
AST_T* ast_new_boolean_literal(int value);
AST_T* ast_new_binary_expression(char op, AST_T* left, AST_T* right);
AST_T* ast_new_unary_expression(char op, AST_T* operand);
AST_T* ast_new_comparison_expression(const char* op, AST_T* left, AST_T* right);
AST_T* ast_new_logical_expression(char op, AST_T* left, AST_T* right);
AST_T* ast_new_variable_reference(const char* name);
AST_T* ast_new_say_statement(AST_T* expression);
AST_T* ast_new_ask_statement(const char* name);
AST_T* ast_new_if_statement(AST_T* condition, AST_T* then_block, AST_T* else_block);
AST_T* ast_new_repeat_statement(AST_T* count_expression, AST_T* body);
AST_T* ast_new_while_statement(AST_T* condition, AST_T* body);
AST_T* ast_new_function_declaration(const char* name, char** parameter_names, size_t parameter_count, AST_T* body);
AST_T* ast_new_function_call(const char* name, AST_T** args, size_t arg_count);
AST_T* ast_new_return_statement(AST_T* expression);
void ast_free(AST_T* node);

#endif