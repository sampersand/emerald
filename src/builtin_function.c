#include "builtin_function.h"
#include "value.h"
#include <time.h>
#include <unistd.h>

void init_builtin_functions(void) {
	// the `srandom` function provides much better random numbers, but isn't technically standard.
#ifdef SRANDOM_UNDEFINED
	srand(time(NULL));
#else
	srandom(time(NULL));
#endif
}

value call_builtin_function(
	const builtin_function *builtin_func,
	unsigned number_of_arguments,
	const value *arguments
) {
	if (builtin_func->required_argument_count != number_of_arguments) {
		die_with_stacktrace("argument mismatch, %s expected %d, but got %d",
			builtin_func->name, builtin_func->required_argument_count, number_of_arguments);
	}

	return (builtin_func->function_pointer)(arguments);
}

void dump_builtin_function(FILE *out, const builtin_function *builtin_func) {
	fprintf(out, "BuiltinFunction(%s)\n", builtin_func->name);
}

static value builtin_to_num_fn(const value *arguments) {
	if (!is_string(arguments[0]))
		die_with_stacktrace("Can only convert strings to numbers, not %s", value_name(arguments[0]));

	return new_number_value(string_to_number(as_string(arguments[0])));
}

static value builtin_prompt_fn(const value *arguments) {
	(void) arguments;

	size_t capacity = 0;
	ssize_t length;
	char *line = NULL;

	// TODO: use fgets instead
	if ((length = getline(&line, &capacity, stdin)) == -1) {
		assert(line != NULL);
		free(line);

		if (!feof(stdin))
			die_with_stacktrace("unable to read line from stdin");

		return new_string_value(new_string(strdup(""), 0));
	}

	assert(0 < length);
	assert(line != NULL);

	// strip trialing newlines
	if (length != 0 && line[length - 1] == '\n') {
		length--;
		if (length != 0 && line[length - 1] == '\r')
			length--;
	}

	value ret = new_string_value(new_string(strndup(line, length), length));
	free(line);

	return ret;
}

static value builtin_print_fn(const value *arguments) {
	string *to_print = value_to_string(arguments[0]);

	printf("%.*s", to_print->length, to_print->ptr);
	fflush(stdout);

	free_string(to_print);
	return VALUE_NULL;
}

static value builtin_println_fn(const value *arguments) {
	builtin_print_fn(arguments);

	putchar('\n');

	return VALUE_NULL;
}

static value builtin_random_fn(const value *arguments) {
	(void) arguments;
	number ret;

#ifdef SRANDOM_UNDEFINED
	ret = rand();
#else
	ret = random();
#endif

	return new_number_value(ret);
}

static value builtin_length_fn(const value *arguments) {
	switch (classify(arguments[0])) {
	case VALUE_KIND_ARRAY:
		return new_number_value(as_array(arguments[0])->length);

	case VALUE_KIND_STRING:
		return new_number_value(as_string(arguments[0])->length);

	default:
		die_with_stacktrace("can only get the length of arrays and strings, not %s", value_name(arguments[0]));
	}
}

static value builtin_exit_fn(const value *arguments) {
	if (!is_number(arguments[0]))
		die_with_stacktrace("can only exit with an integer status code, not %s", value_name(arguments[0]));

	exit(as_number(arguments[0]));
}

static value builtin_dump_fn(const value *arguments) {
	dump_value(stdout, arguments[0]);
	putchar('\n');

	return clone_value(arguments[0]);
}

static value builtin_delete_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		die_with_stacktrace("can only `delete` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		die_with_stacktrace("index needs to be an integer for `delete`, not %s", value_name(arguments[1]));

	value ret = delete_at_array(as_array(arguments[0]), as_number(arguments[1]));
	return ret == VALUE_UNDEFINED ? VALUE_NULL : ret;
}

static value builtin_insert_fn(const value *arguments) {
	if (!is_array(arguments[0]))
		die_with_stacktrace("can only `insert` from arrays, not %s", value_name(arguments[0]));

	if (!is_number(arguments[1]))
		die_with_stacktrace("index needs to be an integer for `insert`, not %s", value_name(arguments[1]));

	if (!insert_at_array(as_array(arguments[0]), as_number(arguments[1]), clone_value(arguments[2]))) {
		die_with_stacktrace("cannot insert to negative indicies larger than `ary`'s length: %lld", as_number(arguments[0]));
	}

	return clone_value(arguments[0]);
}

static value builtin_typeof_fn(const value *arguments) {
	const char *typename = value_name(arguments[0]);

	return new_string_value(new_string(strdup(typename), strlen(typename)));
}

static value builtin_sleep_fn(const value *arguments) {
	if (!is_number(arguments[0]))
		die_with_stacktrace("can only sleep for number seconds");

	return new_number_value(sleep(as_number(arguments[0])));
}

builtin_function builtin_functions[] = {
	{
		.name = "to_ring",
		.required_argument_count = 1,
		.function_pointer = builtin_to_num_fn
	},
	{
		.name = "sotellme",
		.required_argument_count = 0,
		.function_pointer = builtin_prompt_fn
	},
	{
		.name = "gottago",
		.required_argument_count = 1,
		.function_pointer = builtin_print_fn
	},
	{
		.name = "gottagofast",
		.required_argument_count = 1,
		.function_pointer = builtin_println_fn
	},
	{
		.name = "chaos",
		.required_argument_count = 0,
		.function_pointer = builtin_random_fn
	},
	{
		.name = "shoe_size",
		.required_argument_count = 1,
		.function_pointer = builtin_length_fn
	},
	{
		.name = "falloffthetrack",
		.required_argument_count = 1,
		.function_pointer = builtin_exit_fn
	},
	{
		.name = "amy",
		.required_argument_count = 1,
		.function_pointer = builtin_dump_fn
	},
	{
		.name = "buhbyenow",
		.required_argument_count = 2,
		.function_pointer = builtin_delete_fn
	},
	{
		.name = "hereitgoes",
		.required_argument_count = 3,
		.function_pointer = builtin_insert_fn
	},
	{
		.name = "species",
		.required_argument_count = 1,
		.function_pointer = builtin_typeof_fn
	},
	{
		.name = "imwaiting",
		.required_argument_count = 1,
		.function_pointer = builtin_sleep_fn
	},
};

