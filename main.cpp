#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

#define MAX_CAPTURE_COUNT 4

struct Value;

using Box = Value*;

struct Fun {
	int capture_count;
	Box captures[MAX_CAPTURE_COUNT];

	int segment;
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

	static Value Fun(int segment, int capture_count, Box* captures) {
		Value result;
		result.tag = Tag::Fun;
		result.as_fun.segment = segment;
		result.as_fun.capture_count = capture_count;
		for (int i = 0; i < capture_count; ++i) {
			result.as_fun.captures[i] = captures[i];
		}
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
	PushFun,
	Call,
	Return,
	Declare,
	Undeclare,
};

struct VirtualMachine {
	bool is_halted = false;
	std::vector<Value> temp;
	std::vector<Box> env;

	std::vector<std::vector<Bytecode>> segments;

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

	void push_fun(int segment, int capture_count) {
		Box captures[MAX_CAPTURE_COUNT];
		for (int i = 0; i < capture_count; ++i) {
			captures[i] = temp[temp.size() - capture_count + i].as_box;
		}
		temp.push_back(Value::Fun(segment, capture_count, captures));
	}

	void call(int arg_count) {

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

	void declare() {
		Box b = new Value;
		*b = Value::Int(0);
		env.push_back(b);
	}

	void undeclare() {
		env.pop_back();
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
};

void ejemplo_fibonacci() {
	VirtualMachine vm;

	Box a = new Value; *a = Value::Int(0);
	Box b = new Value; *b = Value::Int(1);
	Box c = new Value; *c = Value::Int(0);
	Box n = new Value; *n = Value::Int(8);

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

	for (int i = 0; i < code.size(); ++i) {
		if (i != 0) putchar(i % 16 == 0 ? '\n' : ' ');
		printf("0x%02x", (unsigned int)(unsigned char)code[i]);
	}
	printf("\n");

	vm.segments.push_back(std::move(code));

	vm.interpret(&vm.segments[0][0]);

	std::cout << b->as_int << "\n";
}

void ejemplo_funcion() {
	VirtualMachine vm;

	Box y = new Value; *y = Value::Int(3);
	vm.env.push_back(y);

	std::vector<Bytecode> seg0;
	seg0.push_back(Bytecode(Opcode::Access));
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(Bytecode(Opcode::PushFun));
	seg0.push_back(1); // segment 1
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(1); // 1 capture
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(Bytecode(Opcode::PushInt));
	seg0.push_back(2);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(Bytecode(Opcode::Call));
	seg0.push_back(1);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(0);
	seg0.push_back(Bytecode(Opcode::Halt));

	std::vector<Bytecode> seg1;
	seg1.push_back(Bytecode(Opcode::Access));
	seg1.push_back(0);
	seg1.push_back(0);
	seg1.push_back(0);
	seg1.push_back(0);
	seg1.push_back(Bytecode(Opcode::Read));
	seg1.push_back(Bytecode(Opcode::Access));
	seg1.push_back(1);
	seg1.push_back(0);
	seg1.push_back(0);
	seg1.push_back(0);
	seg1.push_back(Bytecode(Opcode::Read));
	seg1.push_back(Bytecode(Opcode::Add));
	seg1.push_back(Bytecode(Opcode::Return));

	vm.segments.push_back(seg0);
	vm.segments.push_back(seg1);

	vm.interpret(&vm.segments[0][0]);

	std::cout << vm.temp.back().as_int << "\n";
}

int main() {
	ejemplo_funcion();
}
