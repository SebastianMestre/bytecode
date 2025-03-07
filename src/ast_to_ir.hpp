#pragma once

#include <vector>
#include <string>

namespace Ir {
struct Expr;
struct Stmt;
}

namespace Ast {
struct Expr;
struct Stmt;
}

Ir::Expr* ast_to_ir(Ast::Expr* e, std::vector<std::string> names = {});
Ir::Stmt* ast_to_ir(Ast::Stmt* e, std::vector<std::string> names = {});
