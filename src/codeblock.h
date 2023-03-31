#pragma once

#include "bytecode.h"
#include "valuedefn.h"

#define CODEBLOCK_RETURN_LOCAL 0

typedef struct {
	unsigned number_of_locals, code_length, number_of_constants;
	bytecode *code;
	value *constants;
} codeblock;

codeblock *new_codeblock(
	unsigned number_of_locals,
	unsigned code_length,
	bytecode *code,
	unsigned number_of_constants,
	value *constants
);

value run_codeblock(const codeblock *block, unsigned number_of_arguments, const value *arguments);
void free_codeblock(codeblock *block);
