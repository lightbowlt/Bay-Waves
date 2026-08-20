#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* dup_string(const char* value) {
    size_t len = value == NULL ? 0 : strlen(value);
    char* copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    if (len > 0) {
        memcpy(copy, value, len);
    }
    copy[len] = '\0';
    return copy;
}

IRModule ir_build_module(AST_T* program, const char* target_name) {
    IRModule module = { 0, NULL };
    (void)program;
    (void)target_name;
    module.is_valid = 1;
    module.output_path = dup_string("bay_output.bin");
    return module;
}

void ir_free_module(IRModule* module) {
    if (module == NULL) {
        return;
    }
    free(module->output_path);
    module->output_path = NULL;
    module->is_valid = 0;
}
