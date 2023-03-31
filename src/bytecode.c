#include "bytecode.h"

const char *opcode_repr(opcode op) {
	switch (op) {
	case OPCODE_MOVE:          return "MOVE";
	case OPCODE_ARRAY_LITERAL: return "ARRAY_LITERAL";

	case OPCODE_LOAD_CONSTANT:         return "LOAD_CONSTANT";
	case OPCODE_LOAD_GLOBAL_VARIABLE:  return "LOAD_GLOBAL_VARIABLE";
	case OPCODE_STORE_GLOBAL_VARIABLE: return "STORE_GLOBAL_VARIABLE";

	case OPCODE_JUMP:          return "JUMP";
	case OPCODE_JUMP_IF_TRUE:  return "JUMP_IF_TRUE";
	case OPCODE_JUMP_IF_FALSE: return "JUMP_IF_FALSE";
	case OPCODE_CALL:          return "CALL";
	case OPCODE_RETURN:        return "RETURN";

	case OPCODE_NOT:      return "NOT";
	case OPCODE_NEGATE:   return "NEGATE";
	case OPCODE_ADD:      return "ADD";
	case OPCODE_SUBTRACT: return "SUBTRACT";
	case OPCODE_MULTIPLY: return "MULTIPLY";
	case OPCODE_DIVIDE:   return "DIVIDE";
	case OPCODE_MODULO:   return "MODULO";

	case OPCODE_EQUAL:                 return "EQUAL";
	case OPCODE_NOT_EQUAL:             return "NOT_EQUAL";
	case OPCODE_LESS_THAN:             return "LESS_THAN";
	case OPCODE_LESS_THAN_OR_EQUAL:    return "LESS_THAN_OR_EQUAL";
	case OPCODE_GREATER_THAN:          return "GREATER_THAN";
	case OPCODE_GREATER_THAN_OR_EQUAL: return "GREATER_THAN_OR_EQUAL";

	case OPCODE_INDEX:        return "INDEX";
	case OPCODE_INDEX_ASSIGN: return "INDEX_ASSIGN";
	}
}
