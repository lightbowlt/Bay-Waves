#ifndef BAY_RUNTIME_H
#define BAY_RUNTIME_H

#include <stddef.h>

typedef enum {
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        long integer_value;
        char* string_value;
    } as;
} Value;

typedef struct {
    char** names;
    Value* values;
    size_t count;
    size_t capacity;
} Environment;

Environment* environment_new(void);
void environment_free(Environment* env);
void environment_set(Environment* env, const char* name, Value value);
Value environment_get(Environment* env, const char* name);
int environment_has(Environment* env, const char* name);
Value value_integer(long value);
Value value_string(const char* value);
char* value_to_string(Value value);
void value_free(Value value);

#endif
