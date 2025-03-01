#include "virtual_machine.hpp"

void VirtualMachine::push_int(int x) {
	temp.push_back(Value::Int(x));
}

void VirtualMachine::add() {
	int x = temp.back().as_int;
	temp.pop_back();

	int y = temp.back().as_int;
	temp.pop_back();

	push_int(x + y);
}

void VirtualMachine::access(int offset) {
	Box b = env[env.size() - offset - 1];
	temp.push_back(Value::Box(b));
}

void VirtualMachine::read() {
	Box b = temp.back().as_box;
	temp.pop_back();
	temp.push_back(*b);
}

void VirtualMachine::write() {
	Value v = temp.back();
	temp.pop_back();

	Box b = temp.back().as_box;
	temp.pop_back();

	*b = v;
}

void VirtualMachine::jmp(int &pc, int offset) {
	pc += offset;
}

void VirtualMachine::jmp_zero(int &pc, int offset) {
	int value = temp.back().as_int;
	temp.pop_back();
	if (value == 0) {
		jmp(pc, offset);
	}
}

void VirtualMachine::push_fun(int segment, int capture_count) {
	Box captures[MAX_CAPTURE_COUNT];
	for (int i = 0; i < capture_count; ++i) {
		captures[i] = temp[temp.size() - capture_count + i].as_box;
	}
	for (int i = 0; i < capture_count; ++i) {
		temp.pop_back();
	}
	temp.push_back(Value::Fun(segment, capture_count, captures));
}

void VirtualMachine::call(int arg_count) {

	Fun f = temp[temp.size() - arg_count - 1].as_fun;

	for (int i = 0; i < f.capture_count; ++i) {
		env.push_back(f.captures[i]);
	}

	for (int i = 0; i < arg_count; ++i) {
		Box b = new Value;
		*b = temp[temp.size() - arg_count + i];
		env.push_back(b);
	}
	for (int i = 0; i < arg_count; ++i) {
		temp.pop_back();
	}

	// pop function off of temp stack
	temp.pop_back();

	interpret(&segments[f.segment][0]);

	for (int i = 0; i < arg_count; ++i) {
		env.pop_back();
	}
	for (int i = 0; i < f.capture_count; ++i) {
		env.pop_back();
	}
}

void VirtualMachine::declare() {
	Box b = new Value;
	*b = Value::Int(0);
	env.push_back(b);
}

void VirtualMachine::undeclare() {
	env.pop_back();
}

void VirtualMachine::interpret(Bytecode* code) {
	int pc = 0;
	while (true) 
		switch (Opcode(code[pc++])) {
		case Opcode::PushInt: {
			int x;
			memcpy(&x, &code[pc], 4);
			pc += 4;
			push_int(x);
		} break;
		case Opcode::Add: {
			add();
		} break;
		case Opcode::Access: {
			int offset;
			memcpy(&offset, &code[pc], 4);
			pc += 4;
			access(offset);
		} break;
		case Opcode::Read: {
			read();
		} break;
		case Opcode::Write: {
			write();
		} break;
		case Opcode::Jmp: {
			int offset;
			memcpy(&offset, &code[pc], 4);
			pc += 4;
			jmp(pc, offset);
		} break;
		case Opcode::JmpZero: {
			int offset;
			memcpy(&offset, &code[pc], 4);
			pc += 4;
			jmp_zero(pc, offset);
		} break;
		case Opcode::PushFun: {
			int segment;
			memcpy(&segment, &code[pc], 4);
			pc += 4;
			int capture_count;
			memcpy(&capture_count, &code[pc], 4);
			pc += 4;
			push_fun(segment, capture_count);
		} break;
		case Opcode::Call: {
			int arg_count;
			memcpy(&arg_count, &code[pc], 4);
			pc += 4;
			call(arg_count);
			if (is_halted)
				return;
		} break;
		case Opcode::Return: {
			return;
		} break;
		case Opcode::Declare: {
			declare();
		} break;
		case Opcode::Undeclare: {
			undeclare();
		} break;
		case Opcode::Halt: {
			is_halted = true;
			return;
		} break;
	}
}
