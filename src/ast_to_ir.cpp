#include "ast_to_ir.hpp"

#include "abstract_syntax_tree.hpp"
#include "intermediate_representation.hpp"

#include <vector>
#include <cassert>

#include <iostream>

struct NameResolver {

	std::vector<std::string> names;

	int name_to_offset(std::string const& name) {
		for (int offset = 0; offset < int(names.size()); ++offset) {
			int position = int(names.size()) - offset - 1;
			if (names[position] == name) {
				return offset;
			}
		}

		std::cerr << "Nombre no encontrado: " << name << "\n";
		std::cerr << "nombres disponibles:";
		for (auto const& x : names) {
			std::cerr << " " << x;
		}
		std::cerr << "\n";


		// TODO: dar un mensaje de error
		assert(0);
	}


	Ir::Int* convert_expr(Ast::Int* e) {
		return new Ir::Int(e->value);
	}

	Ir::Add* convert_expr(Ast::Add* e) {
		return new Ir::Add(
			convert_expr(e->lhs),
			convert_expr(e->rhs));
	}

	Ir::Fun* convert_expr(Ast::Fun* e) {
		int initial_env_size = int(names.size());

		std::vector<int> captures;
		for (int i = 0; i < int(e->captures.size()); ++i) {
			captures.push_back(name_to_offset(e->captures[i]));
		}

		int argument_count = int(e->parameter_names.size());

		for (int i = 0; i < int(e->captures.size()); ++i) {
			declare(e->captures[i]);
		}

		for (int i = 0; i < int(e->parameter_names.size()); ++i) {
			declare(e->parameter_names[i]);
		}
		
		Ir::Block* body = convert_stmt(e->body);

		while (int(names.size()) > initial_env_size) {
			undeclare();
		}

		return new Ir::Fun(std::move(captures), argument_count, body);
	}

	Ir::Call* convert_expr(Ast::Call* e) {
		Ir::Expr* target = convert_expr(e->target);
		std::vector<Ir::Expr*> arguments;
		for (int i = 0; i < int(e->arguments.size()); ++i) {
			arguments.push_back(convert_expr(e->arguments[i]));
		}
		return new Ir::Call(target, std::move(arguments));
	}

	Ir::Var* convert_expr(Ast::Var* e) {
		return new Ir::Var(name_to_offset(e->name));
	}

	Ir::Expr* convert_expr(Ast::Expr* e) {
		switch (e->tag()) {
		case Ast::Expr::Tag::Int:
			return convert_expr(static_cast<Ast::Int*>(e));
			break;
		case Ast::Expr::Tag::Add:
			return convert_expr(static_cast<Ast::Add*>(e));
			break;
		case Ast::Expr::Tag::Var:
			return convert_expr(static_cast<Ast::Var*>(e));
			break;
		case Ast::Expr::Tag::Fun:
			return convert_expr(static_cast<Ast::Fun*>(e));
			break;
		case Ast::Expr::Tag::Call:
			return convert_expr(static_cast<Ast::Call*>(e));
			break;
		}
		assert(0);
	}


	Ir::While* convert_stmt(Ast::While* e) {
		return new Ir::While(
			convert_expr(e->condition),
			convert_stmt(e->body));
	}

	Ir::Block* convert_stmt(Ast::Block* e) {
		int initial_env_size = int(names.size());

		std::vector<Ir::Stmt*> body;
		for (int i = 0; i < int(e->body.size()); ++i) {
			body.push_back(convert_stmt(e->body[i]));
		}

		while (int(names.size()) > initial_env_size) {
			undeclare();
		}

		return new Ir::Block(std::move(body));
	}

	Ir::Assign* convert_stmt(Ast::Assign* e) {
		return new Ir::Assign(
			name_to_offset(e->name),
			convert_expr(e->value));
	}

	Ir::Declare* convert_stmt(Ast::Declare* e) {
		declare(e->name);
		return new Ir::Declare();
	}

	Ir::Return* convert_stmt(Ast::Return* e) {
		return new Ir::Return(convert_expr(e->value));
	}

	Ir::Stmt* convert_stmt(Ast::Stmt* e) {
		switch (e->tag()) {
		case Ast::Stmt::Tag::While:
			return convert_stmt(static_cast<Ast::While*>(e));
		case Ast::Stmt::Tag::Block:
			return convert_stmt(static_cast<Ast::Block*>(e));
		case Ast::Stmt::Tag::Assign:
			return convert_stmt(static_cast<Ast::Assign*>(e));
		case Ast::Stmt::Tag::Declare:
			return convert_stmt(static_cast<Ast::Declare*>(e));
		case Ast::Stmt::Tag::Return:
			return convert_stmt(static_cast<Ast::Return*>(e));
		}
		assert(0);
	}


	void declare(std::string name) {
		names.push_back(std::move(name));
	}

	void undeclare() {
		names.pop_back();
	}

};

Ir::Expr* ast_to_ir(Ast::Expr* e, std::vector<std::string> names) {
	NameResolver resolver = {std::move(names)};
	return resolver.convert_expr(e);
}

Ir::Stmt* ast_to_ir(Ast::Stmt* e, std::vector<std::string> names) {
	NameResolver resolver = {std::move(names)};
	return resolver.convert_stmt(e);
}
