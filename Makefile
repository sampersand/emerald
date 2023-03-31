ifdef DEBUG
CFLAGS+=-g -fsanitize=address,undefined
endif
CFLAGS += -Wall -Wpedantic -Wextra
CFLAGS += -I.

all: emerald

.PHONY: clean
clean:
	-@rm src/*.o emerald

emerald: src/array.o src/ast.o src/environment.o src/function.o src/main.o src/number.o \
		src/shared.o src/string.o src/token.o src/value.o src/codeblock.o src/compile.o \
		src/bytecode.o src/globals.o src/builtin_function.o
	$(CC) $(CFLAGS) -o $@ $+

*.o: *.c
