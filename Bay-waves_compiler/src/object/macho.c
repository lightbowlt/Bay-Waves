#include "macho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int macho_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name) {
    (void)payload;
    (void)payload_size;
    (void)target_name;
    if (output_path == NULL) {
        fprintf(stderr, "error: missing Mach-O output path\n");
        return 1;
    }

    FILE* out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "error: unable to write Mach-O file '%s'\n", output_path);
        return 1;
    }

    const unsigned char stub[] = {
        0xcf, 0xfa, 0xed, 0xfe, 0x07, 0x00, 0x00, 0x01,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    fwrite(stub, 1, sizeof(stub), out);
    fclose(out);
    return 0;
}
