#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int arm64_codegen_emit(const char* source, unsigned char** out_bytes, size_t* out_size, const char* target_name) {
    (void)source;
    (void)target_name;
    const unsigned char payload[] = { 0xaa, 0xaa, 0xaa, 0xaa };
    *out_size = sizeof(payload);
    *out_bytes = malloc(*out_size);
    if (*out_bytes == NULL) {
        fprintf(stderr, "error: arm64 code generation allocation failed\n");
        return 1;
    }
    memcpy(*out_bytes, payload, *out_size);
    return 0;
}

void arm64_codegen_free(Arm64Program* program) {
    if (program == NULL) {
        return;
    }
    free(program->bytes);
    program->bytes = NULL;
    program->length = 0;
}
