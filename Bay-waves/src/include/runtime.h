#ifndef BAY_RUNTIME_H
#define BAY_RUNTIME_H

#include <stddef.h>
#include "AST.h"

typedef enum {
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_STRING,
    VALUE_TYPE_BOOLEAN,
    VALUE_TYPE_FUNCTION
} ValueType;

typedef struct FunctionValue {
    char* name;
    char** parameter_names;
    size_t parameter_count;
    AST_T* body;
    struct Environment* closure;
} Function;

typedef struct {
    ValueType type;
    union {
        long integer_value;
        char* string_value;
        int boolean_value;
        Function* function_value;
    } as;
} Value;

typedef struct Environment {
    char** names;
    Value* values;
    size_t count;
    size_t capacity;
    struct Environment* parent;
} Environment;

Environment* environment_new(void);
Environment* environment_child(Environment* parent);
void environment_free(Environment* env);
void environment_set(Environment* env, const char* name, Value value);
Value environment_get(Environment* env, const char* name);
int environment_has(Environment* env, const char* name);
Value value_integer(long value);
Value value_boolean(int value);
Value value_string(const char* value);
Value value_function(Function* function);
char* value_to_string(Value value);
void value_free(Value value);

#endif
