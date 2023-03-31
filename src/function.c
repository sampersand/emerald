#include "function.h"
#include "shared.h"
#include <assert.h>
#include <string.h>

function *new_function(
	char *function_name,
	codeblock *body,
	unsigned number_of_arguments,
	char **argument_names,
	unsigned source_line_number,
	const char *source_filename
) {
	assert(strlen(function_name) != 0);

	function *func = xmalloc(sizeof(function));

	func->function_name = function_name;
	func->body = body;
	func->refcount = 1;
	func->number_of_arguments = number_of_arguments;
	func->argument_names = argument_names;
	func->source_line_number = source_line_number;
	func->source_filename = source_filename;

	return func;
}

void deallocate_function(function *func) {
	assert(func->refcount == 0);

	free(func->function_name);
	free_codeblock(func->body);

	for (unsigned i = 0; i < func->number_of_arguments; i++)
		free(func->argument_names[i]);
	free(func->argument_names);

	free(func);
}

value call_function(const function *func, unsigned number_of_arguments, const value *arguments) {
	if (func->number_of_arguments != number_of_arguments) {
		die_with_stacktrace(
			"argument mismatch for %s: expected %d, got %d",
			func->function_name,
			func->number_of_arguments,
			number_of_arguments
		);
	}

	source_code_location location = {
		.filename = func->source_filename,
		.function_name = func->function_name,
		.line_number = func->source_line_number
	};

	enter_stackframe(&location);
	value ret = run_codeblock(func->body, number_of_arguments, arguments);
	leave_stackframe();

	return ret;
}

void dump_function(FILE *out, const function *func) {
	fprintf(out, "Function(%s, args=[", func->function_name);

	for (unsigned i = 0; i < func->number_of_arguments; i++) {
		if (i != 0)
			fputs(", ", out);

		fputs(func->argument_names[i], out);
	}

	fputs("])", out);
}
