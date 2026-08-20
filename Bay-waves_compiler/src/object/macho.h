#ifndef BAY_MACHO_H
#define BAY_MACHO_H

#include <stddef.h>

int macho_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name);

#endif
