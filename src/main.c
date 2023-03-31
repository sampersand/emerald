#include "token.h"
#include "value.h"
#include "compile.h"
#include "codeblock.h"
#include "environment.h"
#include "globals.h"

static void usage(const char *program_name) {
	die("usage: %s (-e 'expression' | -f filename)", program_name);
}

int main(int argc, char **argv) {
	init_environment();
	init_global_variables();
	init_builtin_functions();

	if (argc != 3 || argv[1][0] != '-' || argv[1][1] == '\0' || argv[1][2] != '\0')
		usage(argv[0]);

	switch (argv[1][1]) {
	case 'e': compile("-e", argv[2]); break;
	case 'f': compile(argv[2], read_file(argv[2])); break;
	default: usage(argv[0]);
	}

	int main_index = lookup_global_variable("main");
	if (main_index == GLOBAL_DOESNT_EXIST)
		die("you must define a `main` function");

	value ret = call_value(fetch_global_variable(main_index), 0, NULL);

	free_environment();
	free_global_variables();

	// If the return value of `main` is an integer, that's the return status.
	if (is_number(ret))
		return as_number(ret);

	free_value(ret);
	return 0;
}
