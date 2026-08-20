#ifndef BAY_ARM64_CODEGEN_H
#define BAY_ARM64_CODEGEN_H

#include <stddef.h>

typedef struct {
    unsigned char* bytes;
    size_t length;
} Arm64Program;

int arm64_codegen_emit(const char* source, unsigned char** out_bytes, size_t* out_size, const char* target_name);
void arm64_codegen_free(Arm64Program* program);

#endif
