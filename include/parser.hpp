#ifndef CMINOR_PARSER_HPP
#define CMINOR_PARSER_HPP

#include "ast.hpp"
#include "lexer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cminor {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parse();

    const std::vector<std::string>& errors() const;

private:
    const std::vector<Token>& tokens_;
    std::size_t current_ = 0;
    std::vector<std::string> errors_;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;

    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& message);

    void error(const Token& token, const std::string& message);

    Function parseFunction();
    std::unique_ptr<Stmt> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseDeclaration();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    void synchronize();
};

}

#endif
