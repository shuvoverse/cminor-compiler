#ifndef CMINOR_AST_HPP
#define CMINOR_AST_HPP

#include <memory>
#include <string>
#include <vector>

namespace cminor {

enum class ExprKind {
    Integer,
    Variable,
    Unary,
    Binary,
    Assignment,
    Call
};

struct Expr {
    ExprKind kind;
    std::string value;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::vector<std::unique_ptr<Expr>> arguments;
};

enum class StmtKind {
    Block,
    Declaration,
    Expression,
    Return,
    If,
    While,
    For,
    Break,
    Continue
};

struct Stmt {
    StmtKind kind;
    std::string name;
    std::unique_ptr<Expr> expression;
    std::vector<std::unique_ptr<Stmt>> statements;
    std::unique_ptr<Stmt> elseBranch;
};

struct Function {
    std::string returnType;
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<Stmt> body;
};

struct Program {
    std::vector<Function> functions;
};

}

#endif
