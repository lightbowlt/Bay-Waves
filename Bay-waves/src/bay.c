#include "include/AST.h"
#include "include/bay_lexer.h"
#include "include/bay_parser.h"
#include "include/compiler.h"
#include "include/interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_entire_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "error: unable to open file '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);
    char* buffer = malloc((size_t)length + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, (size_t)length, file);
    fclose(file);
    buffer[read_size] = '\0';
    return buffer;
}

static void print_help(void) {
    printf("Bay v0.1\n");
    printf("Usage:\n");
    printf("  bay run <file.bay>\n");
    printf("  bay compile <file.bay>\n");
    printf("  bay --help\n");
    printf("  bay --version\n");
}

static int run_file(const char* path) {
    char* source = read_entire_file(path);
    if (source == NULL) {
        return 1;
    }

    lexer_T* lexer = init_lexer(source);
    parser_T* parser = init_parser(lexer);
    AST_T* program = parser_parse_program(parser);

    Interpreter* interpreter = interpreter_new();
    interpreter_execute_program(interpreter, program);
    interpreter_free(interpreter);
    ast_free(program);
    free_parser(parser);
    free(source);
    return 0;
}

static int compile_file(const char* path, const char* target_name, const char* output_path) {
    if (path == NULL) {
        fprintf(stderr, "error: missing input file\n");
        return 1;
    }
    return compiler_compile_file(path, target_name, output_path);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        printf("Bay v0.1\n");
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc != 3) {
            print_help();
            return 1;
        }
        return run_file(argv[2]);
    }

    if (strcmp(argv[1], "compile") == 0) {
        if (argc == 3 && strcmp(argv[2], "--help") == 0) {
            compiler_print_help();
            return 0;
        }
        if (argc < 3) {
            compiler_print_help();
            return 1;
        }

        const char* source_path = NULL;
        const char* target_name = NULL;
        const char* output_path = NULL;

        for (int i = 2; i < argc; ++i) {
            if (strcmp(argv[i], "--target") == 0) {
                if (i + 1 >= argc) {
                    compiler_print_help();
                    return 1;
                }
                target_name = argv[++i];
            } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
                if (i + 1 >= argc) {
                    compiler_print_help();
                    return 1;
                }
                output_path = argv[++i];
            } else if (source_path == NULL) {
                source_path = argv[i];
            } else {
                compiler_print_help();
                return 1;
            }
        }

        if (source_path == NULL) {
            compiler_print_help();
            return 1;
        }

        return compile_file(source_path, target_name, output_path);
    }

    fprintf(stderr, "error: unknown command '%s'\n", argv[1]);
    print_help();
    return 1;
}
