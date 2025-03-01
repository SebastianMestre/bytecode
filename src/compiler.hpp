#pragma once

#include "intermediate_representation.hpp"
#include "bytecode_builder.hpp"

struct Compiler {
	BytecodeBuilder bb;

	void compile(Ir::Int* e);
	void compile(Ir::Add* e);
	void compile(Ir::Var* e);
	void compile(Ir::Fun* e);
	void compile(Ir::Call* e);
	void compile(Ir::Expr* e);

	void compile_statement(Ir::While* e);
	void compile_statement(Ir::Block* e);
	void compile_statement(Ir::Assign* e);
	void compile_statement(Ir::Declare* e);
	void compile_statement(Ir::Return* e);
	void compile_statement(Ir::Stmt* e);
};
