#pragma once

#include "valuedefn.h"
#include <stdio.h>

typedef struct {
	VALUE_ALIGNMENT char *name;
	unsigned required_argument_count;
	value (*function_pointer)(const value *arguments);
} builtin_function;

#define NUMBER_OF_BUILTIN_FUNCTIONS 11
extern builtin_function builtin_functions[NUMBER_OF_BUILTIN_FUNCTIONS];

void init_builtin_functions(void);

value call_builtin_function(
	const builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments
);

void dump_builtin_function(FILE *out, const builtin_function *builtin_func);
