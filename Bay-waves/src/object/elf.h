#ifndef BAY_ELF_H
#define BAY_ELF_H

#include <stddef.h>

int elf_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name);

#endif
