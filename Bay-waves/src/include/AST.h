#ifndef AST_H
#define AST_H
#include <ctype.h>
#include <stdlib.h>

typedef struct AST_STRUCT {
    enum {
        AST_VARIABLE_DEFENITION,
        AST_VARIABLE,
        AST_FUNCTION_CALL,
        AST_STRING,
        AST_COMPOND
    } type;

    /* AST VARIABLE DEFINITION */
    char* variable_defenition_variable_name;
    struct AST_STRUCT* variable_defenition_variable_value;

    /* AST VARIABLE */
    char* variable_name;

    /* AST FUNCTION CALL */
    char* function_call_name;
    struct AST_STRUCT** function_call_arguments;
    size_t function_call_arguments_size;

    /* AST STRING */
    char* string_value;

    /* AST COMPOUND */
    struct AST_StRUCT** compound_value;
    size_t compound_size;
} AST_T;

AST_T* init_ast(int type);
#endif