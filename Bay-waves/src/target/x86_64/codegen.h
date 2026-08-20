#ifndef BAY_X86_64_CODEGEN_H
#define BAY_X86_64_CODEGEN_H

#include <stddef.h>

typedef struct {
    unsigned char* bytes;
    size_t length;
} X86_64Program;

int x86_64_codegen_emit(const char* source, unsigned char** out_bytes, size_t* out_size, const char* target_name);
void x86_64_codegen_free(X86_64Program* program);

#endif
