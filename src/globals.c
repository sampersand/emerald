#include "globals.h"
#include "shared.h"
#include "value.h"
#include "builtin_function.h"

typedef struct {
	char *name;
	value val;
} global_variable_entry;

struct {
	unsigned length, capacity;
	global_variable_entry *entries;
} globals;

void init_global_variables(void) {
	globals.length = 0;
	globals.capacity = 8;
	globals.entries = xmalloc(globals.capacity * sizeof(global_variable_entry));

	for (unsigned i = 0; i < NUMBER_OF_BUILTIN_FUNCTIONS; i++) {
		assign_global_variable(
			declare_global_variable(strdup(builtin_functions[i].name)),
			new_builtin_function_value(&builtin_functions[i])
		);
	}
}

void free_global_variables(void) {
	for (unsigned i = 0; i < globals.length; i++) {
		free(globals.entries[i].name);
		free_value(globals.entries[i].val);
	}

	free(globals.entries);
}

int lookup_global_variable(const char *name) {
	for (unsigned i = 0; i < globals.length; i++) {
		if (!strcmp(name, globals.entries[i].name))
			return i;
	}

	return GLOBAL_DOESNT_EXIST;
}

unsigned declare_global_variable(char *name) {
	int previous_index = lookup_global_variable(name);
	if (previous_index != GLOBAL_DOESNT_EXIST)
		return previous_index;

	if (globals.length == globals.capacity) {
		globals.capacity *= 2;
		globals.entries = xrealloc(globals.entries, globals.capacity * sizeof(global_variable_entry));
	}

	unsigned index = globals.length;
	globals.entries[index].name = name;
	globals.entries[index].val = VALUE_NULL;
	globals.length++;
	return index;
}

void assign_global_variable(unsigned index, value val) {
	assert(index < globals.length);

	free_value(globals.entries[index].val);
	globals.entries[index].val = val;
}

value fetch_global_variable(unsigned index) {
	assert(index < globals.length);

	return clone_value(globals.entries[index].val);
}
