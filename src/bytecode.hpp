#pragma once

#include <cstdint>

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
