#include "environment.h"
#include "shared.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

_Thread_local struct {
	unsigned stack_pointer;
	const source_code_location *stackframes[STACKFRAME_LIMIT];
} environment;

void init_environment(void) {
	environment.stack_pointer = 0;
}

void free_environment(void) {
	assert(environment.stack_pointer == 0);
}

void enter_stackframe(const source_code_location *location) {
	// Note this isn't an `die`, as that'd dump hundreds of stackframes.
	// In the future, we may want to only log the first and last few?
	if (environment.stack_pointer == STACKFRAME_LIMIT)
		die("stack level too deep (%d levels deep)", STACKFRAME_LIMIT);

	environment.stackframes[environment.stack_pointer] = location;
	environment.stack_pointer++;
}

void leave_stackframe(void) {
	assert(environment.stack_pointer != 0);

	environment.stack_pointer--;
}

void dump_stacktrace(FILE *out) {
	for (unsigned i = 0; i < environment.stack_pointer; i++) {
		const source_code_location *location = environment.stackframes[i];

		fprintf(out, "%d: %s:%d in %s\n",
			i, location->filename, location->line_number, location->function_name);
	}
}
