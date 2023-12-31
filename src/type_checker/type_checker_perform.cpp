#include "../include/type_checker.h"

void TypeChecker::perform_checks() {
	static_assert(OP_COUNT == 58, "unhandled op types in type_check_program()");
	static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");

	for (auto fn_key = program.functions.begin(); fn_key != program.functions.end(); fn_key++) {
		Function function = fn_key->second;
		std::string func_name = fn_key->first;
		std::map<std::string, LangType> variables;
		std::map<std::string, std::vector<LangType>> label_stack_states;
		std::vector<std::pair<Op, std::vector<LangType>>> jump_op_stack_states;

		// first round of type-checking
		// get all type_stack snapshots of all label sections
		std::vector<LangType> type_stack;
		
		for (LangType t : function.signature.argument_stack)
			type_stack.push_back(t);

		// type check all ops
		for (unsigned long int i = 0; i < function.ops.size(); i++) {
			Op op = function.ops.at(i);

			if (op.type == OP_DUMP) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "dump");
					exit(1);
				}
				type_stack.pop_back();
			}

			// arithmetics
			else if (op.type == OP_PLUS) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "+", "addition");
					exit(1);
				}

				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				// additions goes in following combinations [b, a] -> [b + a]

				// int + int -> int
				if (is_prim_type_int(a) && is_prim_type_int(b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "+", "addition");
					print_note_at_loc(b.loc, "first value pushed here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second value pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_MINUS) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "-", "subtraction");
					exit(1);
				}

				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				// subtraction goes in following combinations [b, a] -> [b - a]

				// int - int -> int
				if (is_prim_type_int(a) && is_prim_type_int(b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "-", "subtraction");
					print_note_at_loc(b.loc, "first value pushed here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second value pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_MUL) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "*", "multiplication");
					exit(1);
				}

				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				// multiplication goes in following combinations

				// int * int -> int
				if (is_prim_type_int(a) && is_prim_type_int(b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "*", "multiplication");
					print_note_at_loc(b.loc, "first value pushed here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second value pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_DIV) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "/", "division");
					exit(1);
				}

				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				// division goes in following combinations

				// int / int -> int, int
				if (is_prim_type_int(a) && is_prim_type_int(b)) {
					type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
					type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
				}
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "/", "division");
					print_note_at_loc(b.loc, "first value pushed here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second value pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}

			// comparisons
			else if (op.type == OP_EQUAL) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "=", "equal to");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "=", "equal to");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_GREATER) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), ">", "greater than");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, ">", "greater than");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_LESS) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "<", "less than");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "<", "less than");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_GREATER_EQ) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), ">=", "greater than or equal to");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, ">=", "greater than or equal to");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_LESS_EQ) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "<=", "less than or equal to");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, ">=", "less than or equal to");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_NOT_EQ) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "!=", "not equal to");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "!=", "not equal to");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_NOT) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "not");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();

				if (is_prim_type_int(a))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "not");
					print_note_at_loc(a.loc, "first argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_AND) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "and");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "and");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}
			else if (op.type == OP_OR) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "or");
					exit(1);
				}
				
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();

				if (types_equal(a, b))
					type_stack.push_back(LangType(
						op.loc, prim_type_name(TYPE_I64), 0
					));
				else {
					print_invalid_combination_of_types_error(op.loc, {b, a}, "or");
					print_note_at_loc(b.loc, "first argument found here (" + human_readable_type(b) + ")");
					print_note_at_loc(a.loc, "second argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}
			}		

			// stack manipulation
			else if (op.type == OP_POP) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "pop");
					exit(1);
				}
				type_stack.pop_back();
			}
			else if (op.type == OP_DUP) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "dup");
					exit(1);
				}
				LangType a = type_stack.back();
				a.loc = op.loc;
				type_stack.push_back(a);
			}
			else if (op.type == OP_SWP) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "swp");
					exit(1);
				}

				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();
				type_stack.push_back(a);
				type_stack.push_back(b);
			}
			else if (op.type == OP_ROT) {
				if (type_stack.size() < 3) {
					print_not_enough_arguments_error(op.loc, 3, type_stack.size(), "rot", "rotate");
					exit(1);
				}

				// [c, b, a] -> [b, a, c]
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();
				LangType c = type_stack.back(); type_stack.pop_back();
				type_stack.push_back(b);
				type_stack.push_back(a);
				type_stack.push_back(c);
			}
			else if (op.type == OP_OVER) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "over");
					exit(1);
				}
				
				// [b, a] -> [b, a, b]
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType b = type_stack.back(); type_stack.pop_back();
				type_stack.push_back(b);
				type_stack.push_back(a);
				b.loc = op.loc;
				type_stack.push_back(b);
			}

			// variables
			else if (op.type == OP_SET_VAR) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "&", "set variable");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType expected_type = function.var_offsets.at(op.str_operand).first;

				if (op.is_prim_type_mode()) {
					if (!types_equal(a, expected_type)) {
						// check in value on stack is a pointer to the said value (a variable)
						expected_type.ptr_to_trace++;
						if (!types_equal(a, expected_type)) {
							expected_type.ptr_to_trace--;
							print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(a), "@", "set variable");
							exit(1);
						}
						op.type = OP_SET_VAR_FROM_OTHER_PTR;
						program.functions.at(func_name).ops.at(i) = op;
					}
				}
				else if (op.mode == MODE_STRUCT) {
					expected_type.ptr_to_trace++;
					// a should be a pointer to the expected_type
					if (a.base_type != expected_type.base_type || a.ptr_to_trace != expected_type.ptr_to_trace) {
						print_error_at_loc(op.loc, "Cannot set '" + op.str_operand + "' to type '" + human_readable_type(a) + "'. Expected type '" + human_readable_type(expected_type) + "'");
						exit(1);
					}
				}
			}
			else if (op.type == OP_SET_VAR_STRUCT_MEMBER) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "@", "set struct member");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				std::pair<LangType, int> member_type_offset = variable_member_offset(op, function.var_offsets, program.structs);
				// the type needs to be a pointer to a struct
				// op-type OP_SET_MEMBER_STRUCT assumes the value was a struct (no pointers)
				if (op.mode == MODE_STRUCT)
					member_type_offset.first.ptr_to_trace += 1;
				if (a.base_type != member_type_offset.first.base_type && a.ptr_to_trace != member_type_offset.first.ptr_to_trace) {
					print_error_at_loc(op.loc, "Cannot set '" + op.str_operand + "' to type '" + human_readable_type(a) + "'. Expected type '" + human_readable_type(member_type_offset.first) + "'");
					exit(1);
				}
			}
			else if (op.type == OP_READ_VAR) {
				// if it is a primitive type sizes as only primitive types can be read directly
				if (op.is_prim_type_mode()) {
					LangType t = function.var_offsets.at(op.str_operand).first;
					t.loc = op.loc;
					type_stack.push_back(t);
				}
				else {
					print_error_at_loc(op.loc, "compiler error: op OP_READ_VAR is set in a non-primitive mode, this is probably a bug in the parser");
					exit(1);
				}
			}
			else if (op.type == OP_READ_VAR_STRUCT_MEMBER) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				std::pair<LangType, int> member_type_offset = variable_member_offset(op, function.var_offsets, program.structs);
				member_type_offset.first.loc = op.loc;
				if (op.mode == MODE_STRUCT)
					member_type_offset.first.ptr_to_trace++;
				type_stack.push_back(member_type_offset.first);
			}
			else if (op.type == OP_PUSH_VAR) {
				type_stack.push_back(LangType(
					op.loc,
					function.var_offsets.at(op.str_operand).first.base_type,
					function.var_offsets.at(op.str_operand).first.ptr_to_trace + 1
				));
			}
			// variable pointers
			else if (op.type == OP_SET_PTR) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "@", "set pointer");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back(); // ptr
				LangType b = type_stack.back(); type_stack.pop_back(); // value
				LangType expected_type(op.loc, op.str_operand);

				expected_type.ptr_to_trace++;
				if (!types_equal(expected_type, a)) {
					print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(a), "@", "set pointer");
					exit(1);
				}
				expected_type.ptr_to_trace--;

				if (op.is_prim_type_mode()) {
					if (!types_equal(b, expected_type)) {
						// check in value on stack is a pointer to the said value (a variable)
						expected_type.ptr_to_trace++;
						if (!types_equal(b, expected_type)) {
							expected_type.ptr_to_trace--;
							print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(b), "@", "set pointer");
							exit(1);
						}
						op.type = OP_SET_PTR_FROM_OTHER_PTR;
						program.functions.at(func_name).ops.at(i) = op;
					}
				}
				else if (op.mode == MODE_STRUCT) {
					expected_type.ptr_to_trace++;
					// a should be a pointer to the expected_type, print error if not
					if (a.base_type != expected_type.base_type || a.ptr_to_trace != expected_type.ptr_to_trace) {
						print_error_at_loc(op.loc, "Cannot set pointer of type '" + op.str_operand + "' to type '" + human_readable_type(a) + "'. Expected type '" + human_readable_type(expected_type) + "'");
						exit(1);
					}
				}
			}
			else if (op.type == OP_SET_PTR_STRUCT_MEMBER) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "@", "set pointer member");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back(); // ptr
				LangType b = type_stack.back(); type_stack.pop_back(); // value
				LangType expected_type(op.loc, split_by_dot(op.str_operand).front());
				
				expected_type.ptr_to_trace++;
				if (!types_equal(expected_type, a)) {
					print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(a), "@", "set pointer member");
					exit(1);
				}
				expected_type.ptr_to_trace--;

				std::pair<LangType, int> member_type_offset = struct_member_offset(op, program.structs);
				if (op.mode == MODE_STRUCT)
					member_type_offset.first.ptr_to_trace++;
				if (!types_equal(b, member_type_offset.first)) {
					print_error_at_loc(op.loc, "cannot set pointer of type '" + op.str_operand + "' to type '" + human_readable_type(b) + "'. Expected type '" + human_readable_type(member_type_offset.first) + "'");
					exit(1);
				}
			}
			else if (op.type == OP_READ_PTR) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "&", "read pointer");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType expected_type(op.loc, op.str_operand);

				expected_type.ptr_to_trace++;
				if (!types_equal(expected_type, a)) {
					print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(a), "&", "read pointer");
					exit(1);
				}
				expected_type.ptr_to_trace--;
				type_stack.push_back(expected_type);
			}
			else if (op.type == OP_READ_PTR_STRUCT_MEMBER) {
				static_assert(MODE_COUNT == 3, "unhandled OpCodeModes in type_check_program()");
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "&", "read pointer member");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				LangType expected_type(op.loc, split_by_dot(op.str_operand).front()); // get type of struct we are getting the member from

				expected_type.ptr_to_trace++;
				if (!types_equal(expected_type, a)) {
					print_invalid_type_error(op.loc, human_readable_type(expected_type), human_readable_type(a), "&", "read pointer member");
					exit(1);
				}
				expected_type.ptr_to_trace--;

				std::pair<LangType, int> member_type_offset = struct_member_offset(op, program.structs);
				member_type_offset.first.loc = op.loc;
				if (op.mode == MODE_STRUCT)
					member_type_offset.first.ptr_to_trace++;
				type_stack.push_back(member_type_offset.first);
			}

			// syscalls
			else if (op.type == OP_SYSCALL0) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "syscall0");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall0");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL1) {
				if (type_stack.size() < 2) {
					print_not_enough_arguments_error(op.loc, 2, type_stack.size(), "syscall1");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall1");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL2) {
				if (type_stack.size() < 3) {
					print_not_enough_arguments_error(op.loc, 3, type_stack.size(), "syscall2");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall2");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL3) {
				if (type_stack.size() < 4) {
					print_not_enough_arguments_error(op.loc, 4, type_stack.size(), "syscall3");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall3");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL4) {
				if (type_stack.size() < 5) {
					print_not_enough_arguments_error(op.loc, 5, type_stack.size(), "syscall4");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall4");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL5) {
				if (type_stack.size() < 6) {
					print_not_enough_arguments_error(op.loc, 6, type_stack.size(), "syscall5");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall5");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_SYSCALL6) {
				if (type_stack.size() < 7) {
					print_not_enough_arguments_error(op.loc, 7, type_stack.size(), "syscall6");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();
				type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "syscall6");
					print_note_at_loc(a.loc, "syscall number pushed here (" + human_readable_type(a) + ")");
					exit(1);
				}
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}

			// labels
			// we do not check OP_JMP and OP_JMPE as they don't consume any data from the stack
			// and we will compare the type stack to the labels in the second loop
			else if (op.type == OP_LABEL) {
				// save current stack state
				label_stack_states.insert({op.str_operand, type_stack});
			}
			else if (op.type == OP_LABEL_END) {
				if (!compare_type_stacks(type_stack, label_stack_states.at(op.str_operand))) {
					print_error_at_loc(op.loc, "different types on stack before and after label definition. types of items on stack must be the same.");
					exit(1);
				}
			}
			else if (op.type == OP_JMP) {
				jump_op_stack_states.push_back({op, type_stack});
			}
			else if (op.type == OP_JMPE) {
				jump_op_stack_states.push_back({op, type_stack});
			}
			else if (op.type == OP_CJMPT) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "cjmpt", "conditional jump if true");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "cjmpt", "conditional jump if true");
					print_note_at_loc(a.loc, "first argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}

				jump_op_stack_states.push_back({op, type_stack});
			}
			else if (op.type == OP_CJMPF) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "cjmpf", "conditional jump if false");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "cjmpf", "conditional jump if false");
					print_note_at_loc(a.loc, "first argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}

				jump_op_stack_states.push_back({op, type_stack});
			}
			else if (op.type == OP_CJMPET) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "cjmpet", "conditional jump to end if true");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "cjmpet", "conditional jump to end if true");
					print_note_at_loc(a.loc, "first argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}

				jump_op_stack_states.push_back({op, type_stack});
			}
			else if (op.type == OP_CJMPEF) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "cjmpef", "conditional jump to end if false");
					exit(1);
				}
				LangType a = type_stack.back(); type_stack.pop_back();

				if (!is_prim_type_int(a)) {
					print_invalid_type_error(op.loc, prim_type_name(TYPE_I64), human_readable_type(a), "cjmpet", "conditional jump to end if false");
					print_note_at_loc(a.loc, "first argument found here (" + human_readable_type(a) + ")");
					exit(1);
				}

				jump_op_stack_states.push_back({op, type_stack});
			}

			// other
			else if (op.type == OP_PUSH_INT) {
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
			}
			else if (op.type == OP_PUSH_STR) {
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I64), 0));
				type_stack.push_back(LangType(op.loc, prim_type_name(TYPE_I8), 1)); // pointer to array of ints (string)
			}
			else if (op.type == OP_PUSH_TYPE_INSTANCE) {
				LangType t(op.loc, op.str_operand);
				t.ptr_to_trace++;
				type_stack.push_back(t);
			}
			else if (op.type == OP_DELETE_PTR) {
				if (type_stack.size() < 1) {
					print_not_enough_arguments_error(op.loc, 1, 0, "delete", "delete pointer");
					exit(1);
				}
				LangType t = type_stack.back(); type_stack.pop_back();
				if (t.ptr_to_trace == 0) {
					print_error_at_loc(op.loc, "Can't delete a non-pointer");
					exit(1);
				}
				op.int_operand = sizeof_type(t, program.structs);
				program.functions.at(func_name).ops.at(i) = op;
			}
			else if (op.type == OP_FUNCTION_CALL) {
				assert(program.functions.count(op.str_operand));

				Function call_func = program.functions.at(op.str_operand);

				if (type_stack.size() < call_func.signature.argument_stack.size()) {
					print_not_enough_arguments_error(op.loc, call_func.signature.argument_stack.size(), type_stack.size(), op.str_operand, "", true);
					exit(1);
				}

				std::vector<LangType> args;
				for (unsigned long int i = call_func.signature.argument_stack.size(); i > 0; i--) {
					args.push_back(type_stack.back());
					type_stack.pop_back();
				}
				std::reverse(args.begin(), args.end());

				bool args_match_types = compare_type_stacks(args, call_func.signature.argument_stack);
				if (!args_match_types) {
					if (call_func.signature.argument_stack.size() == 1)
						print_invalid_type_error(op.loc, human_readable_type(call_func.signature.argument_stack.at(0)), human_readable_type(args.at(0)), op.str_operand, "", true);
					else {
						print_invalid_combination_of_types_error(op.loc, args, op.str_operand, "", true);
						for (LangType t : args)
							print_note_at_loc(t.loc, "argument pushed here (" + human_readable_type(t) + ")");
					}
					exit(1);
				}

				for (LangType t : call_func.signature.return_stack)
					type_stack.push_back(t);
			}

			// unreachable
			else if (op.type == OP_FUN || op.type == OP_END || op.type == OP_COUNT || op.type == OP_STRUCT || op.type == OP_DEFINE_VAR || op.type == OP_CONST || op.type == OP_IMPORT) {
				print_error_at_loc(op.loc, "unreachable: op should be handled in the parsing step. This is probably a bug.");
				exit(1);
			}
		}

		// make sure return values match up with what is specified in the function definition and throw error if not
		if (!compare_type_stacks(type_stack, function.signature.return_stack)) {
			// main cannot have excess data on stack
			if (function.signature.return_stack.size() < type_stack.size()) {
				print_error_at_loc(function.loc, "unhandled data on the stack (expected " + std::to_string(function.signature.return_stack.size()) + " items, got " + std::to_string(type_stack.size()) + ")");

				std::vector<LangType> excess_stack;
				for (unsigned long int i = type_stack.size() - function.signature.return_stack.size(); i > 0; i--) {
					excess_stack.push_back(type_stack.back());
					type_stack.pop_back();
				}
				std::reverse(excess_stack.begin(), excess_stack.end());

				for (LangType t : excess_stack)
					print_note_at_loc(t.loc, "excess data pushed here (" + human_readable_type(t) + ")");
			}

			else if (function.signature.return_stack.size() > type_stack.size())
				print_error_at_loc(function.loc, "not enough data on the stack (expected " + std::to_string(function.signature.return_stack.size()) + " items, got " + std::to_string(type_stack.size()) + ")");

			else if (type_stack.size() > 1) {
				print_invalid_combination_of_types_error(function.loc, type_stack, func_name, "", true);
				for (LangType t : type_stack)
					print_note_at_loc(t.loc, "argument pushed here (" + human_readable_type(t) + ")");
			}

			else {
				print_invalid_type_error(function.loc, human_readable_type(function.signature.return_stack.at(0)), human_readable_type(type_stack.at(0)), func_name, "", true);
				print_note_at_loc(type_stack.at(0).loc, "value pushed here (" + human_readable_type(type_stack.at(0)) + ")");
			}

			exit(1);
		}

		// second round of type-checking
		// go through code again and check jmp labels to see if the stack values are the same as the label they are jumping to
		for (std::pair<Op, std::vector<LangType>> op_stack_pair : jump_op_stack_states) {
			Op op = op_stack_pair.first;
			std::vector<LangType> op_stack_state = op_stack_pair.second;

			if (!label_stack_states.count(op.str_operand)) {
				print_error_at_loc(op.loc, "error found in parser or type-checker. Label '" + op.str_operand + " either doesn't exist or hasn't been found by the parser.");
				exit(1);
			}

			if (!compare_type_stacks(op_stack_state, label_stack_states.at(op.str_operand))) {
				print_error_at_loc(op.loc, "different types on stack then what was expected at the label.");
				exit(1);
			}
		}
	}
}
