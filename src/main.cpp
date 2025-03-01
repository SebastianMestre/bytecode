#include "bytecode_builder.hpp"
#include "compiler.hpp"
#include "intermediate_representation.hpp"
#include "virtual_machine.hpp"

#include <iostream>
#include <vector>

#include <cstdint>
#include <cstring>

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
