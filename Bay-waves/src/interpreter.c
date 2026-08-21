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

static Value evaluate_expression(Interpreter* interpreter, AST_T* node);
static void execute_statement(Interpreter* interpreter, AST_T* node);

static int truthy_value(Value value) {
    if (value.type == VALUE_TYPE_BOOLEAN) {
        return value.as.boolean_value;
    }
    if (value.type == VALUE_TYPE_INTEGER) {
        return value.as.integer_value != 0;
    }
    if (value.type == VALUE_TYPE_STRING) {
        return value.as.string_value != NULL && strcmp(value.as.string_value, "") != 0;
    }
    return 0;
}

static Value evaluate_function_call(Interpreter* interpreter, AST_T* node) {
    if (node == NULL || node->type != AST_FUNCTION_CALL) {
        return value_string("");
    }

    if (!environment_has(interpreter->env, node->function_name)) {
        char message[256];
        snprintf(message, sizeof(message), "undefined function '%s'", node->function_name);
        runtime_error(message);
    }

    Value function_value = environment_get(interpreter->env, node->function_name);
    if (function_value.type != VALUE_TYPE_FUNCTION) {
        char message[256];
        snprintf(message, sizeof(message), "'%s' is not a function", node->function_name);
        runtime_error(message);
    }

    Function* function = function_value.as.function_value;
    if (node->argument_count != function->parameter_count) {
        char message[256];
        snprintf(message, sizeof(message), "function '%s' expects %zu arguments but received %zu", node->function_name, function->parameter_count, node->argument_count);
        runtime_error(message);
    }

    Environment* scope = environment_child(function->closure == NULL ? interpreter->env : function->closure);
    if (scope == NULL) {
        runtime_error("failed to create function scope");
    }

    for (size_t i = 0; i < function->parameter_count; ++i) {
        Value value = evaluate_expression(interpreter, node->arguments[i]);
        environment_set(scope, function->parameter_names[i], value);
    }

    Environment* previous = interpreter->env;
    interpreter->env = scope;
    int returned = 0;
    Value result = value_string("");
    for (size_t i = 0; i < function->body->statement_count; ++i) {
        AST_T* stmt = function->body->statements[i];
        if (stmt == NULL) {
            continue;
        }
        if (stmt->type == AST_RETURN_STATEMENT) {
            result = evaluate_expression(interpreter, stmt->return_value);
            returned = 1;
            break;
        }
        if (stmt->type == AST_IF_STATEMENT) {
            Value condition = evaluate_expression(interpreter, stmt->condition);
            if (truthy_value(condition)) {
                for (size_t j = 0; j < stmt->then_block->statement_count; ++j) {
                    AST_T* inner = stmt->then_block->statements[j];
                    if (inner != NULL && inner->type == AST_RETURN_STATEMENT) {
                        result = evaluate_expression(interpreter, inner->return_value);
                        returned = 1;
                        break;
                    }
                    execute_statement(interpreter, inner);
                }
            } else if (stmt->else_block != NULL) {
                for (size_t j = 0; j < stmt->else_block->statement_count; ++j) {
                    AST_T* inner = stmt->else_block->statements[j];
                    if (inner != NULL && inner->type == AST_RETURN_STATEMENT) {
                        result = evaluate_expression(interpreter, inner->return_value);
                        returned = 1;
                        break;
                    }
                    execute_statement(interpreter, inner);
                }
            }
            if (returned) {
                break;
            }
            continue;
        }
        execute_statement(interpreter, stmt);
    }
    interpreter->env = previous;
    environment_free(scope);
    if (returned) {
        return result;
    }
    return value_string("");
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
        case AST_BOOLEAN_LITERAL:
            return value_boolean(node->boolean_value);
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
        case AST_UNARY_EXPRESSION: {
            Value operand = evaluate_expression(interpreter, node->operand);
            if (node->unary_operator == '!') {
                return value_boolean(!truthy_value(operand));
            }
            runtime_error("unsupported unary operator");
            return value_string("");
        }
        case AST_COMPARISON_EXPRESSION: {
            Value left = evaluate_expression(interpreter, node->left);
            Value right = evaluate_expression(interpreter, node->right);
            int result = 0;
            if (left.type == VALUE_TYPE_INTEGER && right.type == VALUE_TYPE_INTEGER) {
                long a = left.as.integer_value;
                long b = right.as.integer_value;
                if (strcmp(node->comparison_operator, "==") == 0) result = (a == b);
                else if (strcmp(node->comparison_operator, "!=") == 0) result = (a != b);
                else if (strcmp(node->comparison_operator, ">") == 0) result = (a > b);
                else if (strcmp(node->comparison_operator, "<") == 0) result = (a < b);
                else if (strcmp(node->comparison_operator, ">=") == 0) result = (a >= b);
                else if (strcmp(node->comparison_operator, "<=") == 0) result = (a <= b);
            } else {
                char* left_str = value_to_string(left);
                char* right_str = value_to_string(right);
                if (strcmp(node->comparison_operator, "==") == 0) result = (strcmp(left_str, right_str) == 0);
                else if (strcmp(node->comparison_operator, "!=") == 0) result = (strcmp(left_str, right_str) != 0);
                else if (strcmp(node->comparison_operator, ">") == 0) result = (strcmp(left_str, right_str) > 0);
                else if (strcmp(node->comparison_operator, "<") == 0) result = (strcmp(left_str, right_str) < 0);
                else if (strcmp(node->comparison_operator, ">=") == 0) result = (strcmp(left_str, right_str) >= 0);
                else if (strcmp(node->comparison_operator, "<=") == 0) result = (strcmp(left_str, right_str) <= 0);
                free(left_str);
                free(right_str);
            }
            return value_boolean(result);
        }
        case AST_LOGICAL_EXPRESSION: {
            Value left = evaluate_expression(interpreter, node->left);
            Value right = evaluate_expression(interpreter, node->right);
            if (node->logical_operator == 'a') {
                return value_boolean(truthy_value(left) && truthy_value(right));
            }
            if (node->logical_operator == 'o') {
                return value_boolean(truthy_value(left) || truthy_value(right));
            }
            runtime_error("unsupported logical operator");
            return value_string("");
        }
        case AST_FUNCTION_CALL:
            return evaluate_function_call(interpreter, node);
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
        case AST_ASK_STATEMENT: {
            char input[512] = {0};
            if (fgets(input, sizeof(input), stdin) == NULL) {
                input[0] = '\0';
            }
            size_t len = strlen(input);
            while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
                input[--len] = '\0';
            }
            environment_set(interpreter->env, node->ask_name, value_string(input));
            break;
        }
        case AST_IF_STATEMENT: {
            Value condition = evaluate_expression(interpreter, node->condition);
            if (truthy_value(condition)) {
                for (size_t i = 0; i < node->then_block->statement_count; ++i) {
                    execute_statement(interpreter, node->then_block->statements[i]);
                }
            } else if (node->else_block != NULL) {
                for (size_t i = 0; i < node->else_block->statement_count; ++i) {
                    execute_statement(interpreter, node->else_block->statements[i]);
                }
            }
            break;
        }
        case AST_REPEAT_STATEMENT: {
            Value value = evaluate_expression(interpreter, node->count_expression);
            if (value.type != VALUE_TYPE_INTEGER) {
                runtime_error("repeat count must be an integer");
            }
            long count = value.as.integer_value;
            for (long i = 0; i < count; ++i) {
                for (size_t j = 0; j < node->body->statement_count; ++j) {
                    execute_statement(interpreter, node->body->statements[j]);
                }
            }
            break;
        }
        case AST_WHILE_STATEMENT: {
            while (1) {
                Value condition = evaluate_expression(interpreter, node->condition);
                if (!truthy_value(condition)) {
                    break;
                }
                for (size_t i = 0; i < node->body->statement_count; ++i) {
                    execute_statement(interpreter, node->body->statements[i]);
                }
            }
            break;
        }
        case AST_FUNCTION_DECLARATION: {
            Function* function = calloc(1, sizeof(Function));
            function->name = dup_string(node->function_name);
            function->parameter_count = node->parameter_count;
            function->body = node->body;
            function->closure = interpreter->env;
            function->parameter_names = calloc(node->parameter_count, sizeof(char*));
            for (size_t i = 0; i < node->parameter_count; ++i) {
                function->parameter_names[i] = dup_string(node->parameter_names[i]);
            }
            environment_set(interpreter->env, node->function_name, value_function(function));
            break;
        }
        case AST_FUNCTION_CALL: {
            evaluate_expression(interpreter, node);
            break;
        }
        case AST_RETURN_STATEMENT:
            runtime_error("return outside of function");
            break;
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
