#include "pe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pe_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name) {
    (void)payload;
    (void)payload_size;
    (void)target_name;
    if (output_path == NULL) {
        fprintf(stderr, "error: missing PE output path\n");
        return 1;
    }

    FILE* out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "error: unable to write PE file '%s'\n", output_path);
        return 1;
    }

    const unsigned char stub[] = {
        0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    fwrite(stub, 1, sizeof(stub), out);
    fclose(out);
    return 0;
}
