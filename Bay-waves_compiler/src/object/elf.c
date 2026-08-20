#include "elf.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
    (void)payload;
    (void)payload_size;
    (void)target_name;
    if (output_path == NULL) {
        fprintf(stderr, "error: missing ELF output path\n");
        return 1;
    }

    static const char text[] = "hello\n";
    unsigned char code[] = {
        0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,
        0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,
        0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x48, 0xc7, 0xc2, 0x06, 0x00, 0x00, 0x00,
        0x0f, 0x05,
        0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,
        0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00,
        0x0f, 0x05
    };

    const size_t file_size = 0x2000;
    const size_t code_offset = 0x1000;
    const size_t text_offset = 0x1100;
    const uint64_t text_addr = 0x400000ULL + (uint64_t)text_offset;
    const uint64_t entry_addr = 0x400000ULL + (uint64_t)code_offset;

    unsigned char buf[8192] = {0};
    memset(buf, 0, sizeof(buf));

    buf[0x00] = 0x7f; buf[0x01] = 'E'; buf[0x02] = 'L'; buf[0x03] = 'F';
    buf[0x04] = 2;      /* ELFCLASS64 */
    buf[0x05] = 1;      /* ELFDATA2LSB */
    buf[0x06] = 1;      /* EV_CURRENT */
    buf[0x07] = 0;      /* ABI */
    for (int i = 8; i < 16; ++i) {
        buf[i] = 0;
    }

    write_le16(buf + 16, 2);    /* ET_EXEC */
    write_le16(buf + 18, 62);   /* EM_X86_64 */
    write_le32(buf + 20, 1);    /* EV_CURRENT */
    write_le64(buf + 24, entry_addr);
    write_le64(buf + 32, 64);   /* e_phoff */
    write_le64(buf + 40, 0);    /* e_shoff */
    write_le32(buf + 48, 0);    /* e_flags */
    write_le16(buf + 52, 64);   /* e_ehsize */
    write_le16(buf + 54, 56);   /* e_phentsize */
    write_le16(buf + 56, 1);    /* e_phnum */
    write_le16(buf + 58, 0);    /* e_shentsize */
    write_le16(buf + 60, 0);    /* e_shnum */
    write_le16(buf + 62, 0);    /* e_shstrndx */

    write_le32(buf + 64, 1);    /* PT_LOAD */
    write_le32(buf + 68, 5);    /* PF_R | PF_X */
    write_le64(buf + 72, 0);    /* p_offset */
    write_le64(buf + 80, 0x400000ULL);
    write_le64(buf + 88, 0x400000ULL);
    write_le64(buf + 96, file_size);
    write_le64(buf + 104, file_size);
    write_le64(buf + 112, 0x1000ULL);

    memcpy(buf + code_offset, code, sizeof(code));
    memcpy(buf + text_offset, text, sizeof(text));
    write_le64(buf + code_offset + 0x0d, text_addr);

    FILE* out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "error: unable to write ELF file '%s'\n", output_path);
        return 1;
    }

    if (fwrite(buf, 1, file_size, out) != file_size) {
        fclose(out);
        fprintf(stderr, "error: failed to write ELF file '%s'\n", output_path);
        return 1;
    }

    fclose(out);
    chmod(output_path, 0755);
    return 0;
}
