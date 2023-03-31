#include "number.h"
#include <ctype.h>

string *number_to_string(number num) {
	// Assuming 64-bit `long long`, the largest string possible is `-9223372036854775807` (LLONG_MIN)
	// That's 20 characters long. Including the ending `\0`, the buffer should be 21 bytes long. But,
	// to be safe, let's just allocate 255.
	char buf[255];

	// We use `snprintf` just in case `long long`s aren't 64 bits for some reason
	snprintf(buf, sizeof(buf), "%lld", num);

	return new_string(strdup(buf), strlen(buf));
}

// We can't use `strtoll` as strings aren't null terminated.
number string_to_number(const string *str) {
	const char *ptr = str->ptr;
	unsigned index = 0;

	// Remove leading whitespace.
	while (index < str->length && isspace(ptr[index]))
		index++;

	// If there's nothing left, it's 0.
	if (index == str->length)
		return 0;

	// Check for leading `-` or `+`s.
	bool is_negative = ptr[index] == '-';
	if (is_negative || ptr[index] == '+')
		index++;

	// Build the number.
	number num = 0;
	while (index < str->length && isdigit(ptr[index])) {
		num = num * 10 + (ptr[index] - '0');
		index++;
	}

	return is_negative ? -num : num;
}
