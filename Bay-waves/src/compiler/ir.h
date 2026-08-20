#ifndef BAY_IR_H
#define BAY_IR_H

#include "../include/AST.h"

typedef struct {
    int is_valid;
    char* output_path;
} IRModule;

IRModule ir_build_module(AST_T* program, const char* target_name);
void ir_free_module(IRModule* module);

#endif
