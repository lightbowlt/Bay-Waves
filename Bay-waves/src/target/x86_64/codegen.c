#include "codegen.h"
#include "../../include/AST.h"
#include "../../include/bay_lexer.h"
#include "../../include/bay_parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAYLOAD_BASE_ADDR 0x400000ULL + 120ULL

typedef struct {
    char* name;
    char* text;
    long integer_value;
    int is_string;
} Binding;

typedef struct {
    char* text;
    size_t len;
} PrintMessage;

static char* dup_string(const char* value) {
    size_t len = value == NULL ? 0 : strlen(value);
    char* copy = malloc(len + 1U);
    if (copy == NULL) {
        return NULL;
    }
    if (len > 0U) {
        memcpy(copy, value, len);
    }
    copy[len] = '\0';
    return copy;
}

static void append_bytes(unsigned char** out, size_t* out_len, const unsigned char* src, size_t count) {
    if (count == 0U) {
        return;
    }
    unsigned char* resized = realloc(*out, *out_len + count);
    if (resized == NULL) {
        fprintf(stderr, "error: x86_64 code generation allocation failed\n");
        exit(1);
    }
    *out = resized;
    memcpy(*out + *out_len, src, count);
    *out_len += count;
}

static void emit_load_imm64(unsigned char** out, size_t* out_len, int reg_index, uint64_t value) {
    unsigned char bytes[10];
    bytes[0] = 0x48;
    bytes[1] = (unsigned char)(0xB8 + reg_index);
    for (int i = 0; i < 8; ++i) {
        bytes[2 + i] = (unsigned char)((value >> (i * 8)) & 0xffU);
    }
    append_bytes(out, out_len, bytes, sizeof(bytes));
}

static void emit_write_message(unsigned char** out, size_t* out_len, uint64_t msg_addr, size_t msg_len) {
    emit_load_imm64(out, out_len, 0, 1ULL);
    emit_load_imm64(out, out_len, 7, 1ULL);
    emit_load_imm64(out, out_len, 6, msg_addr);
    emit_load_imm64(out, out_len, 2, (uint64_t)msg_len);
    append_bytes(out, out_len, (const unsigned char*)"\x0f\x05", 2U);
}

static void emit_exit_message(unsigned char** out, size_t* out_len) {
    emit_load_imm64(out, out_len, 0, 60ULL);
    append_bytes(out, out_len, (const unsigned char*)"\x48\x31\xff", 3U);
    append_bytes(out, out_len, (const unsigned char*)"\x0f\x05", 2U);
}

static char* format_integer(long value) {
    char buffer[64];
    size_t len = 0U;
    long tmp = value;

    if (value < 0) {
        buffer[len++] = '-';
        tmp = -value;
    }
    if (tmp == 0) {
        buffer[len++] = '0';
    } else {
        char digits[32];
        size_t digit_count = 0U;
        while (tmp > 0) {
            digits[digit_count++] = (char)('0' + (tmp % 10));
            tmp /= 10;
        }
        while (digit_count > 0U) {
            buffer[len++] = digits[--digit_count];
        }
    }
    buffer[len++] = '\n';
    buffer[len] = '\0';
    return dup_string(buffer);
}

static int find_binding(const Binding* bindings, size_t binding_count, const char* name) {
    for (size_t i = 0; i < binding_count; ++i) {
        if (bindings[i].name != NULL && strcmp(bindings[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int evaluate_value(const AST_T* node, const Binding* bindings, size_t binding_count, char** out_text, long* out_int, int* out_is_string) {
    if (node == NULL) {
        return 1;
    }

    switch (node->type) {
        case AST_INTEGER_LITERAL:
            if (out_int != NULL) *out_int = node->integer_value;
            if (out_is_string != NULL) *out_is_string = 0;
            if (out_text != NULL) *out_text = NULL;
            return 0;

        case AST_STRING_LITERAL:
            if (out_text != NULL) *out_text = dup_string(node->string_value);
            if (out_int != NULL) *out_int = 0;
            if (out_is_string != NULL) *out_is_string = 1;
            return 0;

        case AST_VARIABLE_REFERENCE: {
            int index = find_binding(bindings, binding_count, node->variable_name);
            if (index < 0) {
                fprintf(stderr, "error: unknown variable '%s' during compilation\n", node->variable_name);
                return 1;
            }
            if (out_text != NULL) {
                if (bindings[index].is_string) {
                    *out_text = dup_string(bindings[index].text);
                } else {
                    *out_text = NULL;
                }
            }
            if (out_int != NULL) *out_int = bindings[index].integer_value;
            if (out_is_string != NULL) *out_is_string = bindings[index].is_string;
            return 0;
        }

        case AST_BINARY_EXPRESSION: {
            char* left_text = NULL;
            char* right_text = NULL;
            long left_int = 0;
            long right_int = 0;
            int left_is_string = 0;
            int right_is_string = 0;

            if (evaluate_value(node->left, bindings, binding_count, &left_text, &left_int, &left_is_string) != 0) {
                return 1;
            }
            if (evaluate_value(node->right, bindings, binding_count, &right_text, &right_int, &right_is_string) != 0) {
                free(left_text);
                return 1;
            }

            if (left_is_string || right_is_string || node->binary_operator == '+') {
                size_t left_len = left_text != NULL ? strlen(left_text) : 0U;
                size_t right_len = right_text != NULL ? strlen(right_text) : 0U;
                char* combined = malloc(left_len + right_len + 1U);
                if (combined == NULL) {
                    free(left_text);
                    free(right_text);
                    fprintf(stderr, "error: memory allocation failed during compile-time evaluation\n");
                    return 1;
                }
                memcpy(combined, left_text != NULL ? left_text : "", left_len);
                memcpy(combined + left_len, right_text != NULL ? right_text : "", right_len);
                combined[left_len + right_len] = '\0';
                free(left_text);
                free(right_text);
                if (out_text != NULL) *out_text = combined;
                else free(combined);
                if (out_int != NULL) *out_int = 0;
                if (out_is_string != NULL) *out_is_string = 1;
                return 0;
            }

            long result = 0;
            switch (node->binary_operator) {
                case '+': result = left_int + right_int; break;
                case '-': result = left_int - right_int; break;
                case '*': result = left_int * right_int; break;
                case '/':
                    if (right_int == 0) {
                        fprintf(stderr, "error: division by zero during compilation\n");
                        free(left_text);
                        free(right_text);
                        return 1;
                    }
                    result = left_int / right_int;
                    break;
                default:
                    fprintf(stderr, "error: unsupported binary operator '%c'\n", node->binary_operator);
                    free(left_text);
                    free(right_text);
                    return 1;
            }
            free(left_text);
            free(right_text);
            if (out_int != NULL) *out_int = result;
            if (out_is_string != NULL) *out_is_string = 0;
            if (out_text != NULL) *out_text = NULL;
            return 0;
        }

        default:
            return 1;
    }
}

int x86_64_codegen_emit(const char* source, unsigned char** out_bytes, size_t* out_size, const char* target_name) {
    (void)target_name;

    if (source == NULL || out_bytes == NULL || out_size == NULL) {
        return 1;
    }

    char* source_copy = dup_string(source);
    lexer_T* lexer = init_lexer(source_copy);
    parser_T* parser = init_parser(lexer);
    AST_T* program = parser_parse_program(parser);
    if (program == NULL) {
        fprintf(stderr, "error: failed to parse Bay source for x86_64 code generation\n");
        free(source_copy);
        free_parser(parser);
        free_lexer(lexer);
        return 1;
    }

    Binding* bindings = NULL;
    size_t binding_count = 0U;
    PrintMessage* messages = NULL;
    size_t message_count = 0U;

    for (size_t i = 0; i < program->statement_count; ++i) {
        AST_T* statement = program->statements[i];
        if (statement == NULL) {
            continue;
        }

        if (statement->type == AST_VARIABLE_DECLARATION) {
            char* value_text = NULL;
            long value_int = 0;
            int value_is_string = 0;
            if (evaluate_value(statement->declaration_value, bindings, binding_count, &value_text, &value_int, &value_is_string) != 0) {
                fprintf(stderr, "error: failed to evaluate variable '%s' during compilation\n", statement->declaration_name);
                free(value_text);
                free(source_copy);
                ast_free(program);
                free_parser(parser);
                free_lexer(lexer);
                free(bindings);
                return 1;
            }

            bindings = realloc(bindings, sizeof(Binding) * (binding_count + 1U));
            if (bindings == NULL) {
                fprintf(stderr, "error: x86_64 code generation allocation failed\n");
                free(value_text);
                free(source_copy);
                ast_free(program);
                free_parser(parser);
                free_lexer(lexer);
                return 1;
            }
            bindings[binding_count].name = dup_string(statement->declaration_name);
            bindings[binding_count].text = value_is_string ? value_text : NULL;
            bindings[binding_count].integer_value = value_int;
            bindings[binding_count].is_string = value_is_string;
            binding_count += 1U;
            continue;
        }

        if (statement->type == AST_SAY_STATEMENT) {
            char* value_text = NULL;
            long value_int = 0;
            int value_is_string = 0;
            if (evaluate_value(statement->say_expression, bindings, binding_count, &value_text, &value_int, &value_is_string) != 0) {
                fprintf(stderr, "error: failed to evaluate say expression during compilation\n");
                free(source_copy);
                ast_free(program);
                free_parser(parser);
                free_lexer(lexer);
                free(bindings);
                return 1;
            }

            if (value_is_string) {
                size_t len = strlen(value_text);
                char* message = malloc(len + 2U);
                if (message == NULL) {
                    fprintf(stderr, "error: x86_64 code generation allocation failed\n");
                    free(value_text);
                    free(source_copy);
                    ast_free(program);
                    free_parser(parser);
                    free_lexer(lexer);
                    free(bindings);
                    return 1;
                }
                memcpy(message, value_text, len);
                message[len] = '\n';
                message[len + 1U] = '\0';
                free(value_text);
                value_text = message;
            } else {
                value_text = format_integer(value_int);
            }

            messages = realloc(messages, sizeof(PrintMessage) * (message_count + 1U));
            if (messages == NULL) {
                fprintf(stderr, "error: x86_64 code generation allocation failed\n");
                free(value_text);
                free(source_copy);
                ast_free(program);
                free_parser(parser);
                free_lexer(lexer);
                free(bindings);
                return 1;
            }
            messages[message_count].text = value_text;
            messages[message_count].len = strlen(value_text);
            message_count += 1U;
        }
    }

    size_t code_len = 0U;
    size_t data_len = 0U;
    for (size_t i = 0; i < message_count; ++i) {
        code_len += 42U;
        data_len += messages[i].len;
    }
    code_len += 15U;

    unsigned char* out = malloc(code_len + data_len + 16U);
    if (out == NULL) {
        fprintf(stderr, "error: x86_64 code generation allocation failed\n");
        free(source_copy);
        ast_free(program);
        free_parser(parser);
        free_lexer(lexer);
        free(bindings);
        free(messages);
        return 1;
    }

    size_t out_len = 0U;
    size_t data_offset = 0U;
    for (size_t i = 0; i < message_count; ++i) {
        uint64_t msg_addr = PAYLOAD_BASE_ADDR + (uint64_t)(code_len + data_offset);
        emit_write_message(&out, &out_len, msg_addr, messages[i].len);
        data_offset += messages[i].len;
    }
    emit_exit_message(&out, &out_len);

    for (size_t i = 0; i < message_count; ++i) {
        append_bytes(&out, &out_len, (const unsigned char*)messages[i].text, messages[i].len);
    }

    *out_bytes = out;
    *out_size = out_len;

    for (size_t i = 0; i < message_count; ++i) {
        free(messages[i].text);
    }
    free(messages);
    for (size_t i = 0; i < binding_count; ++i) {
        free(bindings[i].name);
        free(bindings[i].text);
    }
    free(bindings);
    free(source_copy);
    ast_free(program);
    free_parser(parser);
    free_lexer(lexer);
    return 0;
}

void x86_64_codegen_free(X86_64Program* program) {
    if (program == NULL) {
        return;
    }
    free(program->bytes);
    program->bytes = NULL;
    program->length = 0;
}
