#pragma once

#include "bytecode.hpp"

#include <vector>

#include <cstring>

struct BytecodeBuilder {
	std::vector<std::vector<Bytecode>> segments;
	int current_segment;

	int begin_segment();
	int end_segment(int old_segment);

	void push_int(int value);
	void add();

	void read();
	void write();
	void declare();
	void access(int offset);

	int label();
	int jmp();
	int jmp_zero();
	void patch(int jmp_address, int label);

	void push_fun(int segment, int capture_count);
	void call(int arg_count);

	void return_op();
	void halt();

private:
	void opcode(Opcode op);

	void immediate(int value);
};
