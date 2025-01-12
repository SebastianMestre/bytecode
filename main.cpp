#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

struct Value;

using Box = Value*;

struct Fun {
};

struct Value {
	enum class Tag { Int, Fun, Box };
	Tag tag;
	union {
		int as_int;
		Fun as_fun;
		Box as_box;
	};

	static Value Int(int x) {
		Value result;
		result.tag = Tag::Int;
		result.as_int = x;
		return result;
	}

	static Value Box(Box x) {
		Value result;
		result.tag = Tag::Box;
		result.as_box = x;
		return result;
	}
};

using Bytecode = uint8_t;

enum class Opcode : Bytecode {
	Halt,
	PushInt,
	Add,
	Access,
	Read,
	Write,
	Jmp,
	JmpZero,
};

struct VirtualMachine {
	std::vector<Value> temp;
	std::vector<Box> env;

	void push_int(int x) {
		temp.push_back(Value::Int(x));
	}

	void add() {
		int x = temp.back().as_int;
		temp.pop_back();

		int y = temp.back().as_int;
		temp.pop_back();

		push_int(x + y);
	}

	void access(int offset) {
		Box b = env[env.size() - offset - 1];
		temp.push_back(Value::Box(b));
	}

	void read() {
		Box b = temp.back().as_box;
		temp.pop_back();
		temp.push_back(*b);
	}

	void write() {
		Value v = temp.back();
		temp.pop_back();

		Box b = temp.back().as_box;
		temp.pop_back();

		*b = v;
	}

	void jmp(int &pc, int offset) {
		pc += offset;
	}

	void jmp_zero(int &pc, int offset) {
		int value = temp.back().as_int;
		temp.pop_back();
		if (value == 0) {
			jmp(pc, offset);
		}
	}

	void interpret(Bytecode* code) {
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
			case Opcode::Halt: {
				return;
			} break;
		}
	}
};

int main() {
	VirtualMachine vm;

	Box a = new Value; *a = Value::Int(0);
	Box b = new Value; *b = Value::Int(1);
	Box c = new Value; *c = Value::Int(0);
	Box n = new Value; *n = Value::Int(7);

	vm.env.push_back(n);
	vm.env.push_back(c);
	vm.env.push_back(b);
	vm.env.push_back(a);

	std::vector<Bytecode> code;
	
	// if n == 0 jmp al final
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(3);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::JmpZero));
	code.push_back(78);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);

	// c = a+b
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(2);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(1);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::Add));
	code.push_back(Bytecode(Opcode::Write));

	// a = b
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(1);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::Write));
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(1);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::Write));

	// b = c
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(1);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(2);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::Write));

	// n = n + (-1)
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(3);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Access));
	code.push_back(3);
	code.push_back(0);
	code.push_back(0);
	code.push_back(0);
	code.push_back(Bytecode(Opcode::Read));
	code.push_back(Bytecode(Opcode::PushInt));
	code.push_back(-1);
	code.push_back(-1);
	code.push_back(-1);
	code.push_back(-1);
	code.push_back(Bytecode(Opcode::Add));
	code.push_back(Bytecode(Opcode::Write));

	code.push_back(Bytecode(Opcode::Jmp));
	code.push_back(-89);
	code.push_back(-1);
	code.push_back(-1);
	code.push_back(-1);

	code.push_back(Bytecode(Opcode::Halt));

	vm.interpret(&code[0]);

	// vm.access(0);
	// vm.access(0);
	// vm.read();
	// vm.push_int(1);
	// vm.add();
	// vm.write();

	std::cout << b->as_int << "\n";
}
