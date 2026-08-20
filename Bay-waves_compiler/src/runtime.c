#include "include/runtime.h"
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

Environment* environment_new(void) {
    Environment* env = calloc(1, sizeof(Environment));
    if (env == NULL) {
        return NULL;
    }
    env->capacity = 8;
    env->names = calloc(env->capacity, sizeof(char*));
    env->values = calloc(env->capacity, sizeof(Value));
    if (env->names == NULL || env->values == NULL) {
        free(env->names);
        free(env->values);
        free(env);
        return NULL;
    }
    return env;
}

void environment_free(Environment* env) {
    if (env == NULL) {
        return;
    }
    for (size_t i = 0; i < env->count; ++i) {
        free(env->names[i]);
        if (env->values[i].type == VALUE_TYPE_STRING) {
            free(env->values[i].as.string_value);
        }
    }
    free(env->names);
    free(env->values);
    free(env);
}

static void ensure_capacity(Environment* env) {
    if (env->count < env->capacity) {
        return;
    }
    size_t new_capacity = env->capacity * 2;
    char** new_names = realloc(env->names, new_capacity * sizeof(char*));
    Value* new_values = realloc(env->values, new_capacity * sizeof(Value));
    if (new_names == NULL || new_values == NULL) {
        fprintf(stderr, "error: memory allocation failed while extending environment\n");
        exit(1);
    }
    env->names = new_names;
    env->values = new_values;
    env->capacity = new_capacity;
}

void environment_set(Environment* env, const char* name, Value value) {
    if (env == NULL || name == NULL) {
        return;
    }

    for (size_t i = 0; i < env->count; ++i) {
        if (strcmp(env->names[i], name) == 0) {
            if (env->values[i].type == VALUE_TYPE_STRING && env->values[i].as.string_value != NULL) {
                free(env->values[i].as.string_value);
            }
            env->values[i] = value;
            return;
        }
    }

    ensure_capacity(env);
    env->names[env->count] = dup_string(name);
    env->values[env->count] = value;
    env->count += 1;
}

Value environment_get(Environment* env, const char* name) {
    if (env == NULL || name == NULL) {
        return value_string("");
    }

    for (size_t i = 0; i < env->count; ++i) {
        if (strcmp(env->names[i], name) == 0) {
            return env->values[i];
        }
    }

    return value_string("");
}

int environment_has(Environment* env, const char* name) {
    if (env == NULL || name == NULL) {
        return 0;
    }
    for (size_t i = 0; i < env->count; ++i) {
        if (strcmp(env->names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

Value value_integer(long value) {
    Value v;
    v.type = VALUE_TYPE_INTEGER;
    v.as.integer_value = value;
    return v;
}

Value value_string(const char* value) {
    Value v;
    v.type = VALUE_TYPE_STRING;
    v.as.string_value = dup_string(value == NULL ? "" : value);
    return v;
}

char* value_to_string(Value value) {
    char buffer[128];
    if (value.type == VALUE_TYPE_INTEGER) {
        snprintf(buffer, sizeof(buffer), "%ld", value.as.integer_value);
        return dup_string(buffer);
    }
    if (value.type == VALUE_TYPE_STRING) {
        return dup_string(value.as.string_value == NULL ? "" : value.as.string_value);
    }
    return dup_string("");
}

void value_free(Value value) {
    if (value.type == VALUE_TYPE_STRING) {
        free(value.as.string_value);
    }
}
