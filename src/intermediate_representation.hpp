#pragma once

#include <vector>

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
