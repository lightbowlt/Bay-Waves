#include "include/interpreter.h"
#include <ctype.h>
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

static void runtime_error(const char* message) {
    fprintf(stderr, "error: %s\n", message);
    exit(1);
}

static Value evaluate_expression(Interpreter* interpreter, AST_T* node) {
    if (node == NULL) {
        return value_string("");
    }

    switch (node->type) {
        case AST_INTEGER_LITERAL:
            return value_integer(node->integer_value);
        case AST_STRING_LITERAL:
            return value_string(node->string_value);
        case AST_VARIABLE_REFERENCE: {
            if (!environment_has(interpreter->env, node->variable_name)) {
                char message[256];
                snprintf(message, sizeof(message), "unknown variable '%s'", node->variable_name);
                runtime_error(message);
            }
            return environment_get(interpreter->env, node->variable_name);
        }
        case AST_BINARY_EXPRESSION: {
            Value left = evaluate_expression(interpreter, node->left);
            Value right = evaluate_expression(interpreter, node->right);

            if (left.type == VALUE_TYPE_INTEGER && right.type == VALUE_TYPE_INTEGER) {
                long a = left.as.integer_value;
                long b = right.as.integer_value;
                switch (node->binary_operator) {
                    case '+': return value_integer(a + b);
                    case '-': return value_integer(a - b);
                    case '*': return value_integer(a * b);
                    case '/':
                        if (b == 0) {
                            runtime_error("division by zero");
                        }
                        return value_integer(a / b);
                    default:
                        runtime_error("unsupported binary operator");
                }
            }

            char* left_str = value_to_string(left);
            char* right_str = value_to_string(right);
            if (node->binary_operator == '+') {
                size_t total = strlen(left_str) + strlen(right_str) + 1;
                char* result = malloc(total);
                snprintf(result, total, "%s%s", left_str, right_str);
                Value v = value_string(result);
                free(result);
                free(left_str);
                free(right_str);
                return v;
            }
            free(left_str);
            free(right_str);
            runtime_error("invalid operand types for expression");
            return value_string("");
        }
        default:
            return value_string("");
    }
}

static char* interpolate_string(Interpreter* interpreter, const char* input) {
    if (input == NULL) {
        return dup_string("");
    }

    size_t capacity = strlen(input) + 1;
    char* output = calloc(capacity, sizeof(char));
    size_t out_len = 0;

    for (size_t i = 0; input[i] != '\0'; ++i) {
        if (input[i] == '\\' && (isalnum((unsigned char)input[i + 1]) || input[i + 1] == '_')) {
            size_t start = i + 1;
            size_t end = start;
            while (input[end] != '\0' && (isalnum((unsigned char)input[end]) || input[end] == '_')) {
                end += 1;
            }

            size_t name_len = end - start;
            char name[128];
            if (name_len >= sizeof(name)) {
                name_len = sizeof(name) - 1;
            }
            memcpy(name, input + start, name_len);
            name[name_len] = '\0';

            if (!environment_has(interpreter->env, name)) {
                char message[256];
                snprintf(message, sizeof(message), "unknown variable '%s'", name);
                runtime_error(message);
            }

            char* replacement = value_to_string(environment_get(interpreter->env, name));
            size_t replacement_len = strlen(replacement);
            while (out_len + replacement_len + 1 >= capacity) {
                capacity *= 2;
                char* grown = realloc(output, capacity);
                if (grown == NULL) {
                    free(output);
                    free(replacement);
                    runtime_error("memory allocation failed during interpolation");
                }
                output = grown;
            }
            memcpy(output + out_len, replacement, replacement_len);
            out_len += replacement_len;
            free(replacement);
            i = end - 1;
            continue;
        }

        if (out_len + 2 >= capacity) {
            capacity *= 2;
            char* grown = realloc(output, capacity);
            if (grown == NULL) {
                free(output);
                runtime_error("memory allocation failed during interpolation");
            }
            output = grown;
        }

        output[out_len++] = input[i];
    }

    output[out_len] = '\0';
    return output;
}

static void execute_statement(Interpreter* interpreter, AST_T* node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case AST_VARIABLE_DECLARATION: {
            Value value = evaluate_expression(interpreter, node->declaration_value);
            environment_set(interpreter->env, node->declaration_name, value);
            break;
        }
        case AST_SAY_STATEMENT: {
            Value value = evaluate_expression(interpreter, node->say_expression);
            char* text = value_to_string(value);
            if (value.type == VALUE_TYPE_STRING && strchr(text, '\\') != NULL) {
                char* expanded = interpolate_string(interpreter, text);
                printf("%s\n", expanded);
                free(expanded);
            } else {
                printf("%s\n", text);
            }
            free(text);
            break;
        }
        default:
            break;
    }
}

Interpreter* interpreter_new(void) {
    Interpreter* interpreter = calloc(1, sizeof(Interpreter));
    if (interpreter == NULL) {
        return NULL;
    }
    interpreter->env = environment_new();
    if (interpreter->env == NULL) {
        free(interpreter);
        return NULL;
    }
    return interpreter;
}

void interpreter_free(Interpreter* interpreter) {
    if (interpreter == NULL) {
        return;
    }
    environment_free(interpreter->env);
    free(interpreter);
}

void interpreter_execute_program(Interpreter* interpreter, AST_T* program) {
    if (interpreter == NULL || program == NULL) {
        return;
    }
    for (size_t i = 0; i < program->statement_count; ++i) {
        execute_statement(interpreter, program->statements[i]);
    }
}
