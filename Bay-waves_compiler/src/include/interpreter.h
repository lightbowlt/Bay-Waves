#ifndef BAY_INTERPRETER_H
#define BAY_INTERPRETER_H

#include "AST.h"
#include "runtime.h"

typedef struct {
    Environment* env;
} Interpreter;

Interpreter* interpreter_new(void);
void interpreter_free(Interpreter* interpreter);
void interpreter_execute_program(Interpreter* interpreter, AST_T* program);

#endif
