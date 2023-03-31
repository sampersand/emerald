#pragma once

#include <stdio.h>

#define die_with_stacktrace(...) (\
	fprintf(stderr, __VA_ARGS__), \
	fputs("\nstacktrace:\n", stderr), \
	dump_stacktrace(stderr), \
	exit(1))

#ifndef STACKFRAME_LIMIT
# define STACKFRAME_LIMIT 1000
#endif

typedef struct {
	const char *filename, *function_name;
	unsigned line_number;
} source_code_location;

void init_environment(void);
void free_environment(void);

void enter_stackframe(const source_code_location *location);
void leave_stackframe(void);

void dump_stacktrace(FILE *out);
