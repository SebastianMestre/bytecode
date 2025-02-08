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
		for (int i = 0; i < capture_count; ++i) {
			temp.pop_back();
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


struct BytecodeBuilder {
	std::vector<std::vector<Bytecode>> segments;
	int current_segment;


	int begin_segment() {
		int old_segment = current_segment;
		current_segment = segments.size();
		segments.push_back({});
		return old_segment;
	}

	int end_segment(int old_segment) {
		int new_segment = current_segment;
		current_segment = old_segment;
		return new_segment;
	}


	void push_int(int value) {
		opcode(Opcode::PushInt);
		immediate(value);
	}

	void add() {
		opcode(Opcode::Add);
	}


	void read() {
		opcode(Opcode::Read);
	}

	void write() {
		opcode(Opcode::Write);
	}

	void declare() {
		opcode(Opcode::Declare);
	}

	void access(int offset) {
		opcode(Opcode::Access);
		immediate(offset);
	}



	int label() {
		return segments[current_segment].size();
	}

	int jmp() {
		int result = label();
		opcode(Opcode::Jmp);
		immediate(0xdeadbeef);
		return result;
	}

	int jmp_zero() {
		int result = label();
		opcode(Opcode::JmpZero);
		immediate(0xcafebabe);
		return result;
	}

	void patch(int jmp_address, int label) {
		int imm_address = jmp_address + 1;
		int jmp_origin = jmp_address + 1 + sizeof(int);
		int offset = label - jmp_origin;
		memcpy(&segments[current_segment][imm_address], &offset, sizeof(offset));
	}

	void push_fun(int segment, int capture_count) {
		opcode(Opcode::PushFun);
		immediate(segment);
		immediate(capture_count);
	}

	void call(int arg_count) {
		opcode(Opcode::Call);
		immediate(arg_count);
	}

	void return_op() {
		opcode(Opcode::Return);
	}

	void halt() {
		opcode(Opcode::Halt);
	}

private:
	void opcode(Opcode op) {
		auto& code = segments[current_segment];
		code.push_back(Bytecode(op));
	}

	void immediate(int value) {
		auto& code = segments[current_segment];
		Bytecode bytes[sizeof(value)];
		memcpy(bytes, &value, sizeof(value));
		for (int i = 0; i < sizeof(value); ++i) {
			code.push_back(bytes[i]);
		}
	}
};

void print_bytecode(std::vector<Bytecode> const& code) {
	printf("\n");
	for (int i = 0; i < code.size(); ++i) {
		if (i != 0) putchar(i % 8 == 0 ? '\n' : ' ');
		printf("%02x", (unsigned int)(unsigned char)code[i]);
	}
	printf("\n");
	printf("\n");
}

void ejemplo_fibonacci() {
	VirtualMachine vm;

	BytecodeBuilder bb;

	bb.begin_segment();

	// n
	bb.declare();
	bb.access(0);
	bb.push_int(8);
	bb.write();

	// c
	bb.declare();
	bb.access(0);
	bb.push_int(0);
	bb.write();

	// b
	bb.declare();
	bb.access(0);
	bb.push_int(1);
	bb.write();

	// a
	bb.declare();
	bb.access(0);
	bb.push_int(0);
	bb.write();

	int loop_start = bb.label();

	// if n == 0 jmp al final
	bb.access(3);
	bb.read();
	int cond_jmp = bb.jmp_zero();

	// c = a+b
	bb.access(2);
	bb.access(0);
	bb.read();
	bb.access(1);
	bb.read();
	bb.add();
	bb.write();

	// a = b
	bb.access(0);
	bb.access(1);
	bb.read();
	bb.write();

	// b = c
	bb.access(1);
	bb.access(2);
	bb.read();
	bb.write();

	// n = n + (-1)
	bb.access(3);
	bb.access(3);
	bb.read();
	bb.push_int(-1);
	bb.add();
	bb.write();

	int back_jmp = bb.jmp();

	int loop_end = bb.label();

	bb.patch(back_jmp, loop_start);
	bb.patch(cond_jmp, loop_end);

	bb.halt();

	print_bytecode(bb.segments[0]);

	vm.segments = std::move(bb.segments);

	vm.interpret(&vm.segments[0][0]);

	auto b = vm.env[2];

	std::cout << b->as_int << "\n";
}

void ejemplo_funcion() {
	VirtualMachine vm;

	BytecodeBuilder bb;

	bb.begin_segment();

	bb.declare();
	bb.access(0);
	bb.push_int(3);
	bb.write();

	int s0 = bb.begin_segment();
	bb.access(0);
	bb.read();
	bb.access(1);
	bb.read();
	bb.add();
	bb.return_op();
	int s1 = bb.end_segment(s0);

	bb.access(0);
	bb.push_fun(s1, 1);

	bb.push_int(2);
	bb.call(1);
	bb.halt();

	print_bytecode(bb.segments[0]);
	print_bytecode(bb.segments[1]);

	vm.segments = std::move(bb.segments);

	vm.interpret(&vm.segments[0][0]);

	std::cout << vm.temp.back().as_int << "\n";
}

namespace Ir { // intermediate representation

struct Expr {
	enum class Tag {
		Int,
		Add,
		Var,
		Fun,
		Call,
	};

	Expr(Tag tag)
		: tag_{tag}
	{}

	Tag tag() { return tag_; }
private:
	Tag tag_;
};

struct Int : Expr {
	int value;

	Int(int value_)
		: Expr(Tag::Int)
		, value{value_}
	{}
};

struct Add : Expr {
	Expr* lhs;
	Expr* rhs;

	Add(Expr* lhs_, Expr* rhs_)
		: Expr(Tag::Add)
		, lhs{lhs_}
		, rhs{rhs_}
	{}
};

struct Var : Expr {
	int offset;

	Var(int offset_)
		: Expr(Tag::Var)
		, offset{offset_}
	{}
};

struct Block;

struct Fun : Expr {
	std::vector<int> captures;
	int argument_count;
	Block* body;

	Fun(std::vector<int> captures_, int argument_count_, Block* body_)
		: Expr(Tag::Fun)
		, captures{std::move(captures_)}
		, argument_count{argument_count_}
		, body{body_}
	{}
};

struct Call : Expr {
	Expr* target;
	std::vector<Expr*> arguments;

	Call(Expr* target_, std::vector<Expr*> arguments_)
		: Expr(Tag::Call)
		, target{target_}
		, arguments{std::move(arguments_)}
	{}
};

struct Stmt {
	enum class Tag {
		While,
		Block,
		Assign,
		Declare,
		Return,
	};

	Stmt(Tag tag)
		: tag_{tag}
	{}

	Tag tag() { return tag_; }
private:
	Tag tag_;
};

struct While : Stmt {
	Expr* condition;
	Stmt* body;

	While(Expr* condition_, Stmt* body_)
		: Stmt(Tag::While)
		, condition{condition_}
		, body{body_}
	{}
};

struct Block : Stmt {
	std::vector<Stmt*> body;

	Block(std::vector<Stmt*> body_)
		: Stmt(Tag::Block)
		, body{std::move(body_)}
	{}
};

struct Assign : Stmt {
	int offset;
	Expr* value;

	Assign(int offset_, Expr* value_)
		: Stmt(Tag::Assign)
		, offset{offset_}
		, value{value_}
	{}
};

struct Declare : Stmt {
	Declare()
		: Stmt(Tag::Declare)
	{}
};

struct Return : Stmt {
	Expr* value;

	Return(Expr* value_)
		: Stmt(Tag::Return)
		, value{value_}
	{}
};

} // namespace Ir

struct Compiler {
	BytecodeBuilder bb;

	void compile(Ir::Int* e) {
		bb.push_int(e->value);
	}

	void compile(Ir::Add* e) {
		compile(e->lhs);
		compile(e->rhs);
		bb.add();
	}

	void compile(Ir::Var* e) {
		bb.access(e->offset);
		bb.read();
	}

	void compile(Ir::Fun* e) {
		int s0 = bb.begin_segment();
		compile_statement(e->body);
		int s1 = bb.end_segment(s0);

		for (int capture : e->captures) {
			bb.access(capture);
		}
		bb.push_fun(s1, e->captures.size());
	}

	void compile(Ir::Call* e) {
		compile(e->target);
		for (auto argument : e->arguments) {
			compile(argument);
		}
		bb.call(e->arguments.size());
	}

	void compile(Ir::Expr* e) {
		switch (e->tag()) {
			case Ir::Expr::Tag::Int: return compile(static_cast<Ir::Int*>(e));
			case Ir::Expr::Tag::Add: return compile(static_cast<Ir::Add*>(e));
			case Ir::Expr::Tag::Var: return compile(static_cast<Ir::Var*>(e));
			case Ir::Expr::Tag::Fun: return compile(static_cast<Ir::Fun*>(e));
			case Ir::Expr::Tag::Call: return compile(static_cast<Ir::Call*>(e));
		}
	}

	void compile_statement(Ir::While* e) {
		int loop_start = bb.label();
		compile(e->condition);
		int cond_jmp = bb.jmp_zero();

		compile_statement(e->body);

		int back_jmp = bb.jmp();
		int loop_end = bb.label();

		bb.patch(back_jmp, loop_start);
		bb.patch(cond_jmp, loop_end);
	}

	void compile_statement(Ir::Block* e) {
		for (auto sub : e->body) {
			compile_statement(sub);
		}
	}

	void compile_statement(Ir::Assign* e) {
		bb.access(e->offset);
		compile(e->value);
		bb.write();
	}

	void compile_statement(Ir::Declare* e) {
		bb.declare();
	}

	void compile_statement(Ir::Return* e) {
		compile(e->value);
		bb.return_op();
	}

	void compile_statement(Ir::Stmt* e) {
		switch (e->tag()) {
			case Ir::Stmt::Tag::While: return compile_statement(static_cast<Ir::While*>(e));
			case Ir::Stmt::Tag::Block: return compile_statement(static_cast<Ir::Block*>(e));
			case Ir::Stmt::Tag::Assign: return compile_statement(static_cast<Ir::Assign*>(e));
			case Ir::Stmt::Tag::Declare: return compile_statement(static_cast<Ir::Declare*>(e));
			case Ir::Stmt::Tag::Return: return compile_statement(static_cast<Ir::Return*>(e));
		}
	}
};

void ejemplo_funcion_ir() {
	VirtualMachine vm;

	Compiler cr;
	auto& bb = cr.bb;

	bb.begin_segment();

	cr.compile_statement(new Ir::Block({
		new Ir::Declare(),
		new Ir::Assign(0, new Ir::Int(3)),
	}));

	cr.compile(new Ir::Call(
		new Ir::Fun({0}, 1, new Ir::Block({
			new Ir::Return(new Ir::Add(new Ir::Var(0), new Ir::Var(1))),
		})),
		{new Ir::Int(2)}));

	bb.halt();

	print_bytecode(bb.segments[0]);
	print_bytecode(bb.segments[1]);

	vm.segments = std::move(bb.segments);

	vm.interpret(&vm.segments[0][0]);

	std::cout << vm.temp.back().as_int << "\n";
}

void ejemplo_fibonacci_ir() {
	VirtualMachine vm;

	Compiler cr;
	auto& bb = cr.bb;

	bb.begin_segment();

	cr.compile_statement( new Ir::Block({
		// n
		new Ir::Declare(),
		new Ir::Assign(0, new Ir::Int(8)),

		// c
		new Ir::Declare(),
		new Ir::Assign(0, new Ir::Int(0)),

		// b
		new Ir::Declare(),
		new Ir::Assign(0, new Ir::Int(1)),

		// a
		new Ir::Declare(),
		new Ir::Assign(0, new Ir::Int(0)),

		new Ir::While( new Ir::Var(3),
			new Ir::Block({

				// c = a+b
				new Ir::Assign(2, new Ir::Add( new Ir::Var(0), new Ir::Var(1))),

				// a = b
				new Ir::Assign(0, new Ir::Var(1)),

				// b = c
				new Ir::Assign(1, new Ir::Var(2)),

				// n = n + (-1)
				new Ir::Assign(3, new Ir::Add( new Ir::Var(3), new Ir::Int(-1))),
		}))

	}));

	bb.halt();

	print_bytecode(bb.segments[0]);

	vm.segments = std::move(bb.segments);

	vm.interpret(&vm.segments[0][0]);

	auto b = vm.env[2];

	std::cout << b->as_int << "\n";
}


int main() {
	ejemplo_funcion();
	ejemplo_funcion_ir();

	ejemplo_fibonacci();
	ejemplo_fibonacci_ir();
}
