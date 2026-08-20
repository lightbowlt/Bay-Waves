#ifndef BAY_COMPILER_H
#define BAY_COMPILER_H

#include <stddef.h>

int compiler_compile_file(const char* path, const char* target_name);
void compiler_print_help(void);

#endif
