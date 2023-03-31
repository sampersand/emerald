#include "compile.h"
#include "bytecode.h"
#include "codeblock.h"
#include "shared.h"
#include "function.h"
#include "value.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>

#define parse_error(...) die(__VA_ARGS__)

// Since we discard all locals after returning, we can use the return local as scratch.
#define SCRATCH_LOCAL CODEBLOCK_RETURN_LOCAL

// You could do this with the classic `length/capacity` + `realloc` scheme, but
// it really clutters up the code. And, really, when was the last time you had more
// than 16 nested whiles...
#ifndef MAX_NUMBER_OF_NESTED_WHILES
# define MAX_NUMBER_OF_NESTED_WHILES 16
#endif

#ifndef MAX_NUMBER_OF_BREAKS_PER_WHILE
# define MAX_NUMBER_OF_BREAKS_PER_WHILE 64
#endif

typedef struct {
	char *name;
	unsigned local_index;
} local_variable_entry;

typedef struct {
	struct {
		unsigned length, capacity;
		local_variable_entry *entries;
	} local_variables;

	unsigned number_of_locals;

	struct {
		unsigned length, capacity;
		value *consts;
	} constants;

	struct {
		unsigned length, capacity;
		bytecode *code;
	} bytecode;

	struct {
		unsigned length;
		unsigned start_of_conditions[MAX_NUMBER_OF_NESTED_WHILES];

		struct _each_while_break {
			unsigned length;
			unsigned code_positions[MAX_NUMBER_OF_BREAKS_PER_WHILE];
		} breaks[MAX_NUMBER_OF_NESTED_WHILES];
	} whiles;
} codeblock_builder;

static unsigned next_local_index(codeblock_builder *builder) {
	unsigned local_index = builder->number_of_locals;
	builder->number_of_locals++;
	return local_index;
}

static unsigned declare_local_variable(codeblock_builder *builder, char *name) {
	// Check to see if the variable's been used before
	for (unsigned i = 0; i < builder->local_variables.length; i++) {
		if (!strcmp(builder->local_variables.entries[i].name, name)) {
			free(name);
			// It has, return that index.
			return builder->local_variables.entries[i].local_index;
		}
	}

	// We haven't seen the variable before, let's add it.

	if (builder->local_variables.length == builder->local_variables.capacity) {
		builder->local_variables.capacity *= 2;
		builder->local_variables.entries = xrealloc(
			builder->local_variables.entries,
			builder->local_variables.capacity * sizeof(local_variable_entry)
		);
	}

	unsigned local_index = next_local_index(builder);

	builder->local_variables.entries[builder->local_variables.length].name = name;
	builder->local_variables.entries[builder->local_variables.length].local_index = local_index;

	LOG("locals[%d] = %s\n", local_index, name);

	builder->local_variables.length++;
	return local_index;
}

#define VARIABLE_DOESNT_EXIST (-1)

static int lookup_local_variable(codeblock_builder *builder, const char *name) {
	for (unsigned i = 0; i < builder->local_variables.length; i++) {
		if (!strcmp(builder->local_variables.entries[i].name, name))
			return builder->local_variables.entries[i].local_index;
	}

	return VARIABLE_DOESNT_EXIST;
}

static void set_bytecode(codeblock_builder *builder, bytecode bc) {
	if (builder->bytecode.length == builder->bytecode.capacity) {
		builder->bytecode.capacity *= 2;
		builder->bytecode.code = xrealloc(
			builder->bytecode.code,
			builder->bytecode.capacity * sizeof(bytecode)
		);
	}

	builder->bytecode.code[builder->bytecode.length] = bc;
	builder->bytecode.length++;
}

static void set_opcode(codeblock_builder *builder, opcode op) {
	LOG("code[% 3d] = op(%s)", builder->bytecode.length, opcode_repr(op));
	set_bytecode(builder, (bytecode) { .op = op });
}

static void set_count(codeblock_builder *builder, unsigned count) {
	LOG("code[% 3d] = count(%d)", builder->bytecode.length, count);
	set_bytecode(builder, (bytecode) { .count = count });
}

static void set_local(codeblock_builder *builder, unsigned local) {
	LOG("code[% 3d] = local(%d)", builder->bytecode.length, local);
	set_bytecode(builder, (bytecode) { .count = local });
}

#define DUMMY_COUNT_PLACEHOLDER 0xAABBCCDD
static unsigned defer_jump(codeblock_builder *builder) {
	LOG("code[% 3d] = <defered jump>", builder->bytecode.length);
	unsigned code_position = builder->bytecode.length;
	set_bytecode(builder, (bytecode) { .count = DUMMY_COUNT_PLACEHOLDER });
	return code_position;
}

static void set_jump_dst(codeblock_builder *builder, unsigned jmp_src) {
	assert(builder->bytecode.code[jmp_src].count == DUMMY_COUNT_PLACEHOLDER);
	LOG("code[% 3d] = count(%d) (update)", jmp_src, builder->bytecode.length);
	builder->bytecode.code[jmp_src].count = builder->bytecode.length;
}

static void load_constant(codeblock_builder *builder, value constant, unsigned target_local) {
	unsigned constant_index;

	// If the constant already exists, then we don't need to store it again.
	for (unsigned i = 0; i < builder->constants.length; i++) {
		if (equate_values(builder->constants.consts[i], constant)) {
			constant_index = i;
			free_value(constant);
			goto found_constant;
		}
	}

	// We didn't find it, we need to allocate it.
	if (builder->constants.length == builder->constants.capacity) {
		builder->constants.capacity *= 2;
		builder->constants.consts = xrealloc(
			builder->constants.consts,
			builder->constants.capacity * sizeof(value)
		);
	}

	constant_index = builder->constants.length;
	builder->constants.consts[constant_index] = constant;
	builder->constants.length++;

found_constant:

	set_opcode(builder, OPCODE_LOAD_CONSTANT);
	set_count(builder, constant_index);
	set_local(builder, target_local);
}

static void compile_expression(codeblock_builder *builder, ast_expression *expression, unsigned target_local);
static void compile_primary(codeblock_builder *builder, ast_primary *primary, unsigned target_local) {
	switch (primary->kind) {
	case AST_PRIMARY_PAREN:
		compile_expression(builder, primary->paren.expression, target_local);
		break;

	case AST_PRIMARY_INDEX: {
		unsigned source_local = next_local_index(builder);
		compile_primary(builder, primary->index.source, source_local);
		compile_expression(builder, primary->index.index, target_local);
		set_opcode(builder, OPCODE_INDEX);
		set_local(builder, source_local);
		set_local(builder, target_local);
		set_local(builder, target_local);
		break;
	}

	case AST_PRIMARY_FUNCTION_CALL: {
		unsigned function_local = next_local_index(builder);
		compile_primary(builder, primary->function_call.function, function_local);

		unsigned argument_locals[primary->function_call.number_of_arguments];
		for (unsigned i = 0; i < primary->function_call.number_of_arguments; i++) {
			argument_locals[i] = next_local_index(builder);
			compile_expression(builder, primary->function_call.arguments[i], argument_locals[i]);
		}
		free(primary->function_call.arguments);

		set_opcode(builder, OPCODE_CALL);
		set_local(builder, function_local);
		set_count(builder, primary->function_call.number_of_arguments);

		for (unsigned i = 0; i < primary->function_call.number_of_arguments; i++)
			set_local(builder, argument_locals[i]);

		set_local(builder, target_local);
		break;
	}

	case AST_PRIMARY_UNARY_OPERATOR:
		compile_primary(builder, primary->unary_operator.primary, target_local);
		switch (primary->unary_operator.operator) {
		case UNARY_OP_NEGATE:
			set_opcode(builder, OPCODE_NEGATE); 
			break;

		case UNARY_OP_NOT:
			set_opcode(builder, OPCODE_NOT); 
			break;
		}
		set_local(builder, target_local);
		set_local(builder, target_local);
		break;

	case AST_PRIMARY_ARRAY_LITERAL: {
		unsigned element_locals[primary->array_literal.length];

		for (unsigned i = 0; i < primary->array_literal.length; i++) {
			element_locals[i] = next_local_index(builder);
			compile_expression(builder, primary->array_literal.elements[i], element_locals[i]);
		}
		free(primary->array_literal.elements);

		set_opcode(builder, OPCODE_ARRAY_LITERAL);
		set_count(builder, primary->array_literal.length);

		for (unsigned i = 0; i < primary->array_literal.length; i++)
			set_local(builder, element_locals[i]);

		set_local(builder, target_local);
		break;
	}

	case AST_PRIMARY_VARIABLE: {
		int local_index = lookup_local_variable(builder, primary->variable.name);

		if (local_index != VARIABLE_DOESNT_EXIST) {
			free(primary->variable.name);

			set_opcode(builder, OPCODE_MOVE);
			set_local(builder, local_index);
			set_local(builder, target_local);
			break;
		}

		int global_index = lookup_global_variable(primary->variable.name);

		if (global_index == GLOBAL_DOESNT_EXIST)
			parse_error("undeclared variable '%s'", primary->variable.name);
		free(primary->variable.name);

		set_opcode(builder, OPCODE_LOAD_GLOBAL_VARIABLE);
		set_count(builder, global_index);
		set_local(builder, target_local);
		break;
	}

	case AST_PRIMARY_LITERAL:
		load_constant(builder, primary->literal.val, target_local);
		break;
	}

	free(primary);
}

static opcode binary_operator_to_opcode(binary_operator operator) {
	switch (operator) {
	case BINARY_OP_UNDEF: bug("BINARY_OP_UNDEF outside of an assignment");
	case BINARY_OP_ADD:                   return OPCODE_ADD;
	case BINARY_OP_SUBTRACT:              return OPCODE_SUBTRACT;
	case BINARY_OP_MULTIPLY:              return OPCODE_MULTIPLY;
	case BINARY_OP_DIVIDE:                return OPCODE_DIVIDE;
	case BINARY_OP_MODULO:                return OPCODE_MODULO;
	case BINARY_OP_EQUAL:                 return OPCODE_EQUAL;
	case BINARY_OP_NOT_EQUAL:             return OPCODE_NOT_EQUAL;
	case BINARY_OP_LESS_THAN:             return OPCODE_LESS_THAN;
	case BINARY_OP_LESS_THAN_OR_EQUAL:    return OPCODE_LESS_THAN_OR_EQUAL;
	case BINARY_OP_GREATER_THAN:          return OPCODE_GREATER_THAN;
	case BINARY_OP_GREATER_THAN_OR_EQUAL: return OPCODE_GREATER_THAN_OR_EQUAL;
	}
}

static void compile_expression(codeblock_builder *builder, ast_expression *expression, unsigned target_local) {
	switch (expression->kind) {
	case AST_EXPRESSION_ASSIGN: {
		compile_expression(builder, expression->assign.value, target_local);

		int local_index = lookup_local_variable(builder, expression->assign.name);
		if (local_index != VARIABLE_DOESNT_EXIST) {
			free(expression->assign.name);

			if (expression->assign.operator != BINARY_OP_UNDEF) {
				set_opcode(builder, binary_operator_to_opcode(expression->assign.operator));
				set_local(builder, local_index);
				set_local(builder, target_local);
				set_local(builder, target_local);
			}

			set_opcode(builder, OPCODE_MOVE);
			set_local(builder, target_local);
			set_local(builder, local_index);
			break;
		}

		int global_index = lookup_global_variable(expression->assign.name);
		if (global_index == GLOBAL_DOESNT_EXIST) {
			parse_error("unknown variable '%s'; declare it first.", expression->assign.name);
		}

		free(expression->assign.name);

		if (expression->assign.operator != BINARY_OP_UNDEF) {
			unsigned old_local_index = next_local_index(builder);
			set_opcode(builder, OPCODE_LOAD_GLOBAL_VARIABLE);
			set_count(builder, global_index);
			set_local(builder, old_local_index);

			set_opcode(builder, binary_operator_to_opcode(expression->assign.operator));
			set_local(builder, old_local_index);
			set_local(builder, target_local);
			set_local(builder, old_local_index);
		}

		set_opcode(builder, OPCODE_STORE_GLOBAL_VARIABLE);
		set_local(builder, global_index);
		set_local(builder, target_local);
		set_local(builder, target_local);
		break;
	}

	case AST_EXPRESSION_INDEX_ASSIGN: {
		unsigned source_local = next_local_index(builder);
		unsigned index_local = next_local_index(builder);

		compile_primary(builder, expression->index_assign.source, source_local);
		compile_expression(builder, expression->index_assign.index, index_local);
		compile_expression(builder, expression->index_assign.value, target_local);

		if (expression->index_assign.operator != BINARY_OP_UNDEF) {
			unsigned old_value_local = next_local_index(builder);
			set_opcode(builder, OPCODE_INDEX);
			set_local(builder, source_local);
			set_local(builder, index_local);
			set_local(builder, old_value_local);

			set_opcode(builder, binary_operator_to_opcode(expression->index_assign.operator));
			set_local(builder, old_value_local);
			set_local(builder, target_local);
			set_local(builder, target_local);
		}

		set_opcode(builder, OPCODE_INDEX_ASSIGN);
		set_local(builder, source_local);
		set_local(builder, index_local);
		set_local(builder, target_local);
		set_local(builder, target_local);
		break;
	}

	case AST_EXPRESSION_SHORT_CIRCUIT_OPERATOR: {
		compile_primary(builder, expression->binary_operator.lhs, target_local);

		switch (expression->short_circuit_operator.operator) {
		case SHORT_CIRCUIT_OR_OR:
			set_opcode(builder, OPCODE_JUMP_IF_TRUE);
			break;

		case SHORT_CIRCUIT_AND_AND:
			set_opcode(builder, OPCODE_JUMP_IF_FALSE);
			break;
		}

		set_local(builder, target_local);
		unsigned jump_to_short_circuit_end = defer_jump(builder);
		compile_expression(builder, expression->short_circuit_operator.rhs, target_local);
		set_jump_dst(builder, jump_to_short_circuit_end);
		break;
	}

	case AST_EXPRESSION_BINARY_OPERATOR: {
		unsigned lhs_local = next_local_index(builder);
		compile_primary(builder, expression->binary_operator.lhs, lhs_local);
		compile_expression(builder, expression->binary_operator.rhs, target_local);

		set_opcode(builder, binary_operator_to_opcode(expression->binary_operator.operator));
		set_local(builder, lhs_local);
		set_local(builder, target_local);
		set_local(builder, target_local);
		break;
	}

	case AST_EXPRESSION_PRIMARY:
		compile_primary(builder, expression->primary, target_local);
		break;
	}

	free(expression);
}

static void compile_block(codeblock_builder *builder, ast_block *block);
static void compile_statement(codeblock_builder *builder, ast_statement *statement) {
	switch (statement->kind) {
	case AST_STATEMENT_LOCAL: {
		unsigned new_local = declare_local_variable(builder, statement->local.name);

		if (statement->local.initializer == NULL) {
			load_constant(builder, VALUE_NULL, new_local);
		} else {
			compile_expression(builder, statement->local.initializer, new_local);
		}

		break;
	}

	case AST_STATEMENT_RETURN:
		if (statement->return_.expression == NULL) {
			load_constant(builder, VALUE_NULL, CODEBLOCK_RETURN_LOCAL);
		} else {
			compile_expression(builder, statement->return_.expression, CODEBLOCK_RETURN_LOCAL);
		}

		// the `return` opcode takes no arguments, as it returns the `SCRATCH_LOCAL` argument
		set_opcode(builder, OPCODE_RETURN);
		break;

	case AST_STATEMENT_IF:
		compile_expression(builder, statement->if_.condition, SCRATCH_LOCAL);
		set_opcode(builder, OPCODE_JUMP_IF_FALSE);
		set_local(builder, SCRATCH_LOCAL);
		unsigned if_false_jump = defer_jump(builder);

		compile_block(builder, statement->if_.if_true);

		if (statement->if_.if_false == NULL) {
			set_jump_dst(builder, if_false_jump);
		} else {
			set_opcode(builder, OPCODE_JUMP);
			unsigned if_true_jump_to_end = defer_jump(builder);

			set_jump_dst(builder, if_false_jump);
			compile_block(builder, statement->if_.if_false);

			set_jump_dst(builder, if_true_jump_to_end);
		}
		break;

	case AST_STATEMENT_WHILE: {
		unsigned beginning_of_condition = builder->bytecode.length;
		compile_expression(builder, statement->while_.condition, SCRATCH_LOCAL);
		set_opcode(builder, OPCODE_JUMP_IF_FALSE);
		set_local(builder, SCRATCH_LOCAL);
		unsigned jump_to_while_end = defer_jump(builder);

		if (builder->whiles.length == MAX_NUMBER_OF_NESTED_WHILES)
			parse_error("too many nested whiles encountered; only %d max allowed", MAX_NUMBER_OF_NESTED_WHILES);

		builder->whiles.start_of_conditions[builder->whiles.length] = beginning_of_condition;
		builder->whiles.breaks[builder->whiles.length].length = 0;
		builder->whiles.length++;
		compile_block(builder, statement->while_.body);

		set_opcode(builder, OPCODE_JUMP);
		set_count(builder, beginning_of_condition);
		set_jump_dst(builder, jump_to_while_end);

		builder->whiles.length--;
		for (unsigned i = 0; i < builder->whiles.breaks[builder->whiles.length].length; i++)
			set_jump_dst(builder, builder->whiles.breaks[builder->whiles.length].code_positions[i]);

		break;
	}

	case AST_STATEMENT_BREAK:
		if (builder->whiles.length == 0)
			parse_error("cannot break when not within a while");

		struct _each_while_break *breaks = &builder->whiles.breaks[builder->whiles.length - 1];

		if (breaks->length == MAX_NUMBER_OF_BREAKS_PER_WHILE) {
			parse_error(
				"too many breaks encountered; only %d max allowed per while",
				MAX_NUMBER_OF_BREAKS_PER_WHILE
			);
		}

		set_opcode(builder, OPCODE_JUMP);
		breaks->code_positions[breaks->length] = defer_jump(builder);
		breaks->length++;
		break;

	case AST_STATEMENT_CONTINUE:
		if (builder->whiles.length == 0)
			parse_error("cannot continue when not within a while");

		set_opcode(builder, OPCODE_JUMP);
		set_count(builder, builder->whiles.start_of_conditions[builder->whiles.length - 1]);
		break;

	case AST_STATEMENT_EXPRESSION:
		compile_expression(builder, statement->expression, SCRATCH_LOCAL);
		break;
	}

	free(statement);
}

static void compile_block(codeblock_builder *builder, ast_block *block) {
	for (unsigned i = 0; i < block->number_of_statements; i++)
		compile_statement(builder, block->statements[i]);

	free(block->statements);
	free(block);
}

static function *build_function(
	char *function_name,
	unsigned number_of_arguments,
	char **argument_names,
	ast_block *body,
	const char *source_filename,
	unsigned source_line_number
) {
	codeblock_builder builder;

	builder.local_variables.length = 0;
	builder.local_variables.capacity = 4;
	builder.local_variables.entries = xmalloc(
		builder.local_variables.capacity * sizeof(local_variable_entry)
	);

	builder.number_of_locals = 1; // As we have an initial `CODEBLOCK_RETURN_LOCAL`.

	// Arguments are simply the first few local variables
	for (unsigned i = 0; i < number_of_arguments; i++)
		(void) declare_local_variable(&builder, strdup(argument_names[i]));

	builder.constants.length = 0;
	builder.constants.capacity = 4;
	builder.constants.consts = xmalloc(builder.constants.capacity * sizeof(value));

	builder.bytecode.length = 0;
	builder.bytecode.capacity = 8;
	builder.bytecode.code = xmalloc(builder.bytecode.capacity * sizeof(bytecode));

	builder.whiles.length = 0;

	compile_block(&builder, body);

	// all functions implicitly return `null` at the end.
	load_constant(&builder, VALUE_NULL, CODEBLOCK_RETURN_LOCAL);
	set_opcode(&builder, OPCODE_RETURN);

	for (unsigned i = 0; i < builder.local_variables.length; i++)
		free(builder.local_variables.entries[i].name);
	free(builder.local_variables.entries);

	codeblock *block = new_codeblock(
		builder.number_of_locals,
		builder.bytecode.length,
		builder.bytecode.code,
		builder.constants.length,
		builder.constants.consts
	);

	return new_function(
		function_name,
		block,
		number_of_arguments,
		argument_names,
		source_line_number,
		source_filename
	);
}

static void compile_declaration(ast_declaration *declaration) {
	switch (declaration->kind) {
	case AST_DECLARATION_FUNCTION: {
		// declare it beforehand so recursive functions can reference the defn.
		unsigned global = declare_global_variable(strdup(declaration->function.name));

		value function = new_function_value(build_function(
			declaration->function.name,
			declaration->function.number_of_arguments,
			declaration->function.argument_names,
			declaration->function.body,
			declaration->source.filename,
			declaration->source.line_number
		));

		if (fetch_global_variable(global) != VALUE_NULL)
			parse_error("function %s redefined", declaration->function.name);

		assign_global_variable(global, function);
		break;
	}

	case AST_DECLARATION_IMPORT: {
		// read the file contents
		char *contents = read_file(declaration->import.path);
		compile(declaration->import.path, contents);
		break;
	}


	case AST_DECLARATION_GLOBAL:
		declare_global_variable(declaration->global.name);
		break;
	}

	free(declaration);
}

void compile(const char *filename, const char *source_code) {
	tokenizer tzr = new_tokenizer(filename, source_code);

	while (true) {
		ast_declaration *declaration = next_declaration(&tzr);

		if (declaration == NULL)
			break;

#ifdef ENABLE_LOGGING
		dump_ast_declaration(stdout, declaration);
#endif

		compile_declaration(declaration);
	}
}
