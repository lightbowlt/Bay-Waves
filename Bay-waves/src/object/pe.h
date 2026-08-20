#ifndef BAY_PE_H
#define BAY_PE_H

#include <stddef.h>

int pe_write_executable(const char* output_path, const unsigned char* payload, size_t payload_size, const char* target_name);

#endif
