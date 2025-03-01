#include "bytecode_builder.hpp"

int BytecodeBuilder::begin_segment() {
	int old_segment = current_segment;
	current_segment = segments.size();
	segments.push_back({});
	return old_segment;
}

int BytecodeBuilder::end_segment(int old_segment) {
	int new_segment = current_segment;
	current_segment = old_segment;
	return new_segment;
}


void BytecodeBuilder::push_int(int value) {
	opcode(Opcode::PushInt);
	immediate(value);
}

void BytecodeBuilder::add() {
	opcode(Opcode::Add);
}


void BytecodeBuilder::read() {
	opcode(Opcode::Read);
}

void BytecodeBuilder::write() {
	opcode(Opcode::Write);
}

void BytecodeBuilder::declare() {
	opcode(Opcode::Declare);
}

void BytecodeBuilder::access(int offset) {
	opcode(Opcode::Access);
	immediate(offset);
}



int BytecodeBuilder::label() {
	return segments[current_segment].size();
}

int BytecodeBuilder::jmp() {
	int result = label();
	opcode(Opcode::Jmp);
	immediate(0xdeadbeef);
	return result;
}

int BytecodeBuilder::jmp_zero() {
	int result = label();
	opcode(Opcode::JmpZero);
	immediate(0xcafebabe);
	return result;
}

void BytecodeBuilder::patch(int jmp_address, int label) {
	int imm_address = jmp_address + 1;
	int jmp_origin = jmp_address + 1 + sizeof(int);
	int offset = label - jmp_origin;
	memcpy(&segments[current_segment][imm_address], &offset, sizeof(offset));
}

void BytecodeBuilder::push_fun(int segment, int capture_count) {
	opcode(Opcode::PushFun);
	immediate(segment);
	immediate(capture_count);
}

void BytecodeBuilder::call(int arg_count) {
	opcode(Opcode::Call);
	immediate(arg_count);
}

void BytecodeBuilder::return_op() {
	opcode(Opcode::Return);
}

void BytecodeBuilder::halt() {
	opcode(Opcode::Halt);
}

void BytecodeBuilder::opcode(Opcode op) {
	auto& code = segments[current_segment];
	code.push_back(Bytecode(op));
}

void BytecodeBuilder::immediate(int value) {
	auto& code = segments[current_segment];
	Bytecode bytes[sizeof(value)];
	memcpy(bytes, &value, sizeof(value));
	for (int i = 0; i < sizeof(value); ++i) {
		code.push_back(bytes[i]);
	}
}
