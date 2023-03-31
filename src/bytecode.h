#pragma once

typedef enum {
	OPCODE_MOVE,
	OPCODE_ARRAY_LITERAL,

	OPCODE_LOAD_CONSTANT,
	OPCODE_LOAD_GLOBAL_VARIABLE,
	OPCODE_STORE_GLOBAL_VARIABLE,

	OPCODE_JUMP,
	OPCODE_JUMP_IF_TRUE,
	OPCODE_JUMP_IF_FALSE,
	OPCODE_CALL,
	OPCODE_RETURN,

	OPCODE_NOT,
	OPCODE_NEGATE,
	OPCODE_ADD,
	OPCODE_SUBTRACT,
	OPCODE_MULTIPLY,
	OPCODE_DIVIDE,
	OPCODE_MODULO,
	OPCODE_EQUAL,
	OPCODE_NOT_EQUAL,
	OPCODE_LESS_THAN,
	OPCODE_LESS_THAN_OR_EQUAL,
	OPCODE_GREATER_THAN,
	OPCODE_GREATER_THAN_OR_EQUAL,
	OPCODE_INDEX,
	OPCODE_INDEX_ASSIGN
} opcode;

typedef union {
	opcode op;
	unsigned count;
} bytecode;

const char *opcode_repr(opcode op);
