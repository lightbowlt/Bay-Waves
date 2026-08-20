#include "include/compiler.h"
#include "compiler/ir.h"
#include "target/target.h"
#include "target/x86_64/codegen.h"
#include "target/arm64/codegen.h"
#include "object/elf.h"
#include "object/pe.h"
#include "object/macho.h"
#include "include/bay_lexer.h"
#include "include/bay_parser.h"
#include "include/AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

static void compiler_error(const char* message) {
    fprintf(stderr, "error: %s\n", message);
    exit(1);
}

static void strip_extension(char* dest, size_t dest_size, const char* source_path) {
    const char* last_dot = strrchr(source_path, '.');
    const char* last_slash = strrchr(source_path, '/');
    if (last_dot != NULL && (last_slash == NULL || last_dot > last_slash)) {
        size_t prefix_len = (size_t)(last_dot - source_path);
        if (prefix_len >= dest_size) {
            prefix_len = dest_size - 1;
        }
        memcpy(dest, source_path, prefix_len);
        dest[prefix_len] = '\0';
        return;
    }
    snprintf(dest, dest_size, "%s", source_path);
}

static int write_target_file(const Target* target, const char* output_path, unsigned char* machine_code, size_t machine_size) {
    if (strcmp(target->object_format, "pe") == 0) {
        return pe_write_executable(output_path, machine_code, machine_size, target->name);
    }
    if (strcmp(target->object_format, "elf") == 0) {
        return elf_write_executable(output_path, machine_code, machine_size, target->name);
    }
    if (strcmp(target->object_format, "macho") == 0) {
        return macho_write_executable(output_path, machine_code, machine_size, target->name);
    }

    fprintf(stderr, "error: unsupported object format '%s' for target '%s'\n", target->object_format, target->name);
    return 1;
}

void compiler_print_help(void) {
    printf("Bay compile usage:\n");
    printf("  bay compile <file.bay>\n");
    printf("  bay compile <file.bay> --target <target>\n");
    printf("  bay compile <file.bay> -o <output>\n");
    printf("  bay compile <file.bay> --output <output>\n\n");
    target_print_supported();
}

int compiler_compile_file(const char* path, const char* target_name, const char* output_path) {
    if (path == NULL) {
        fprintf(stderr, "error: missing input file\n");
        return 1;
    }

    Target target = target_parse(target_name == NULL ? target_detect_host() : target_name);
    if (target.kind == TARGET_INVALID) {
        fprintf(stderr, "error: unsupported target '%s'\n", target_name == NULL ? "<auto>" : target_name);
        target_print_supported();
        return 1;
    }

    char* source = read_entire_file(path);
    if (source == NULL) {
        return 1;
    }

    lexer_T* lexer = init_lexer(source);
    parser_T* parser = init_parser(lexer);
    AST_T* program = parser_parse_program(parser);

    if (program == NULL) {
        free(source);
        free_parser(parser);
        free_lexer(lexer);
        compiler_error("failed to parse Bay source");
    }

    IRModule module = ir_build_module(program, target.name);
    if (!module.is_valid) {
        ast_free(program);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        compiler_error("failed to build compiler IR");
    }

    unsigned char* machine_code = NULL;
    size_t machine_size = 0;
    int codegen_ok = 0;

    if (strcmp(target.cpu_name, "x86_64") == 0) {
        codegen_ok = x86_64_codegen_emit(source, &machine_code, &machine_size, target.name);
    } else if (strcmp(target.cpu_name, "arm64") == 0) {
        codegen_ok = arm64_codegen_emit(source, &machine_code, &machine_size, target.name);
    } else {
        fprintf(stderr, "error: unsupported CPU backend '%s' for target '%s'\n", target.cpu_name, target.name);
        codegen_ok = 1;
    }

    char resolved_output[512];
    if (output_path != NULL && output_path[0] != '\0') {
        snprintf(resolved_output, sizeof(resolved_output), "%s", output_path);
    } else {
        char base_path[512];
        strip_extension(base_path, sizeof(base_path), path);
        snprintf(resolved_output, sizeof(resolved_output), "%s", base_path);
    }

    int result = 0;
    if (codegen_ok == 0) {
        result = write_target_file(&target, resolved_output, machine_code, machine_size);
        if (result == 0) {
            printf("compiled %s -> %s\n", path, resolved_output);
        } else {
            unlink(resolved_output);
        }
    }

    free(machine_code);
    ir_free_module(&module);
    ast_free(program);
    free_parser(parser);
    free_lexer(lexer);
    free(source);
    return result;
}
