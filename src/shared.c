#include "shared.h"
#include <errno.h>
#include <string.h>

void *xmalloc(size_t size) {
	void *ptr = malloc(size);

	// If a zero size is given, `malloc` is allowed to return `NULL`.
	if (size != 0 && ptr == NULL) {
		fprintf(stderr, "allocation error, unable to allocate to %zu bytes", size);
		abort();
	}

	return ptr;
}

void *xrealloc(void *ptr, size_t size) {
	ptr = realloc(ptr, size);

	// If a zero size is given, `realloc` is allowed to return `NULL`.
	if (size != 0 && ptr == NULL) {
		fprintf(stderr, "allocation error, unable to allocate to %zu bytes", size);
		abort();
	}

	return ptr;
}

char *read_file(const char *filename) {
	FILE *file = fopen(filename, "r");

	if (file == NULL)
		die("unable to read file '%s': %s", filename, strerror(errno));

	size_t length = 0;
	size_t capacity = 2048;
	char *contents = xmalloc(capacity);

	while (!feof(file)) {
		size_t amntread = fread(&contents[length], 1, capacity - length, file);

		if (amntread == 0) {
			if (!feof(file))
				die("unable to read file '%s': %s'", filename, strerror(errno));
			break;
		}

		length += amntread;

		if (length == capacity) {
			capacity *= 2;
			contents = xrealloc(contents, capacity);
		}
	}

	if (fclose(file) == EOF)
		perror("couldn't close input file");

	contents = xrealloc(contents, length + 1);
	contents[length] = '\0';
	return contents;
}
