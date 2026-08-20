#include "elf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void write_le16(unsigned char* dst, uint16_t value) {
    dst[0] = (unsigned char)(value & 0xffu);
    dst[1] = (unsigned char)((value >> 8) & 0xffu);
}

static void write_le32(unsigned char* dst, uint32_t value) {
    dst[0] = (unsigned char)(value & 0xffu);
    dst[1] = (unsigned char)((value >> 8) & 0xffu);
    dst[2] = (unsigned char)((value >> 16) & 0xffu);
    dst[3] = (unsigned char)((value >> 24) & 0xffu);
}

static void write_le64(unsigned char* dst, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        dst[i] = (unsigned char)((value >> (i * 8)) & 0xffu);
    }
}

int elf_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name) {
    (void)target_name;

    if (output_path == NULL) {
        fprintf(stderr, "error: missing ELF output path\n");
        return 1;
    }
    if (payload == NULL && payload_size != 0) {
        fprintf(stderr, "error: missing ELF payload for '%s'\n", output_path);
        return 1;
    }

    const uint64_t base_addr = 0x400000ULL;
    const size_t header_size = 64;
    const size_t phdr_size = 56;
    const uint64_t entry_addr = base_addr + (uint64_t)(header_size + phdr_size);
    const uint64_t file_size = (uint64_t)header_size + (uint64_t)phdr_size + (uint64_t)payload_size;

    unsigned char* buf = calloc(1, (size_t)file_size + 16);
    if (buf == NULL) {
        fprintf(stderr, "error: unable to allocate ELF buffer for '%s'\n", output_path);
        return 1;
    }

    buf[0x00] = 0x7f; buf[0x01] = 'E'; buf[0x02] = 'L'; buf[0x03] = 'F';
    buf[0x04] = 2;
    buf[0x05] = 1;
    buf[0x06] = 1;
    buf[0x07] = 0;
    for (int i = 8; i < 16; ++i) {
        buf[i] = 0;
    }

    write_le16(buf + 16, 2);
    write_le16(buf + 18, 62);
    write_le32(buf + 20, 1);
    write_le64(buf + 24, entry_addr);
    write_le64(buf + 32, 64);
    write_le64(buf + 40, 0);
    write_le32(buf + 48, 0);
    write_le16(buf + 52, 64);
    write_le16(buf + 54, 56);
    write_le16(buf + 56, 1);
    write_le16(buf + 58, 0);
    write_le16(buf + 60, 0);
    write_le16(buf + 62, 0);

    write_le32(buf + 64, 1);
    write_le32(buf + 68, 7);
    write_le64(buf + 72, 0);
    write_le64(buf + 80, base_addr);
    write_le64(buf + 88, base_addr);
    write_le64(buf + 96, file_size);
    write_le64(buf + 104, file_size);
    write_le64(buf + 112, 0x1000ULL);

    if (payload_size > 0) {
        memcpy(buf + header_size + phdr_size, payload, payload_size);
    }

    FILE* out = fopen(output_path, "wb");
    if (out == NULL) {
        free(buf);
        fprintf(stderr, "error: unable to write ELF file '%s'\n", output_path);
        return 1;
    }

    if (fwrite(buf, 1, (size_t)file_size, out) != (size_t)file_size) {
        fclose(out);
        free(buf);
        unlink(output_path);
        fprintf(stderr, "error: failed to write ELF file '%s'\n", output_path);
        return 1;
    }

    fclose(out);
    free(buf);
    chmod(output_path, 0755);
    return 0;
}
