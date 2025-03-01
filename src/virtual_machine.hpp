#pragma once

#include "bytecode.hpp"
#include "virtual_machine_value.hpp"

#include <vector>

#include <cstring>

struct VirtualMachine {
	bool is_halted = false;
	std::vector<Value> temp;
	std::vector<Box> env;
	std::vector<std::vector<Bytecode>> segments;

	void push_int(int x);
	void add();

	void declare();
	void undeclare();
	void access(int offset);
	void read();
	void write();

	void jmp(int &pc, int offset);
	void jmp_zero(int &pc, int offset);

	void push_fun(int segment, int capture_count);
	void call(int arg_count);

	void interpret(Bytecode* code);
};
