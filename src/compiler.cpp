#include "compiler.hpp"

void Compiler::compile(Ir::Int* e) {
	bb.push_int(e->value);
}

void Compiler::compile(Ir::Add* e) {
	compile(e->lhs);
	compile(e->rhs);
	bb.add();
}

void Compiler::compile(Ir::Var* e) {
	bb.access(e->offset);
	bb.read();
}

void Compiler::compile(Ir::Fun* e) {
	int s0 = bb.begin_segment();
	compile_statement(e->body);
	int s1 = bb.end_segment(s0);

	for (int capture : e->captures) {
		bb.access(capture);
	}
	bb.push_fun(s1, e->captures.size());
}

void Compiler::compile(Ir::Call* e) {
	compile(e->target);
	for (auto argument : e->arguments) {
		compile(argument);
	}
	bb.call(e->arguments.size());
}

void Compiler::compile(Ir::Expr* e) {
	switch (e->tag()) {
		case Ir::Expr::Tag::Int: return compile(static_cast<Ir::Int*>(e));
		case Ir::Expr::Tag::Add: return compile(static_cast<Ir::Add*>(e));
		case Ir::Expr::Tag::Var: return compile(static_cast<Ir::Var*>(e));
		case Ir::Expr::Tag::Fun: return compile(static_cast<Ir::Fun*>(e));
		case Ir::Expr::Tag::Call: return compile(static_cast<Ir::Call*>(e));
	}
}

void Compiler::compile_statement(Ir::While* e) {
	int loop_start = bb.label();
	compile(e->condition);
	int cond_jmp = bb.jmp_zero();

	compile_statement(e->body);

	int back_jmp = bb.jmp();
	int loop_end = bb.label();

	bb.patch(back_jmp, loop_start);
	bb.patch(cond_jmp, loop_end);
}

void Compiler::compile_statement(Ir::Block* e) {
	for (auto sub : e->body) {
		compile_statement(sub);
	}
}

void Compiler::compile_statement(Ir::Assign* e) {
	bb.access(e->offset);
	compile(e->value);
	bb.write();
}

void Compiler::compile_statement(Ir::Declare* e) {
	bb.declare();
}

void Compiler::compile_statement(Ir::Return* e) {
	compile(e->value);
	bb.return_op();
}

void Compiler::compile_statement(Ir::Stmt* e) {
	switch (e->tag()) {
		case Ir::Stmt::Tag::While: return compile_statement(static_cast<Ir::While*>(e));
		case Ir::Stmt::Tag::Block: return compile_statement(static_cast<Ir::Block*>(e));
		case Ir::Stmt::Tag::Assign: return compile_statement(static_cast<Ir::Assign*>(e));
		case Ir::Stmt::Tag::Declare: return compile_statement(static_cast<Ir::Declare*>(e));
		case Ir::Stmt::Tag::Return: return compile_statement(static_cast<Ir::Return*>(e));
	}
}
