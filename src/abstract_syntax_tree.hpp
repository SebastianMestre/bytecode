#pragma once

#include <string>
#include <vector>

namespace Ast { // abstract syntax tree

struct Expr {
	enum class Tag {
		Int,
		Add,
		Var,
		Fun,
		Call
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
	std::string name;

	Var(std::string name_)
		: Expr(Tag::Var)
		, name{std::move(name_)}
	{}
};

struct Block;

struct Fun : Expr {
	std::vector<std::string> captures;
	std::vector<std::string> parameter_names;
	Block* body;

	Fun(std::vector<std::string> captures_, std::vector<std::string> parameter_names_, Block* body_)
		: Expr(Tag::Fun)
		, captures{std::move(captures_)}
		, parameter_names{std::move(parameter_names_)}
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
	std::string name;
	Expr* value;

	Assign(std::string name_, Expr* value_)
		: Stmt(Tag::Assign)
		, name{std::move(name_)}
		, value{value_}
	{}
};

struct Declare : Stmt {
	std::string name;

	Declare(std::string name_)
		: Stmt(Tag::Declare)
		, name{std::move(name_)}
	{}
};

struct Return : Stmt {
	Expr* value;

	Return(Expr* value_)
		: Stmt(Tag::Return)
		, value{value_}
	{}
};

} // namespace Ast
