#include "parser.hpp"

#include <stdexcept>
#include <utility>

namespace cminor {

namespace {

std::unique_ptr<Expr> makeBinary(
    std::string op,
    std::unique_ptr<Expr> left,
    std::unique_ptr<Expr> right
) {
    auto expr = std::make_unique<Expr>();
    expr->kind = ExprKind::Binary;
    expr->value = std::move(op);
    expr->left = std::move(left);
    expr->right = std::move(right);
    return expr;
}

}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens) {}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    return !isAtEnd() && peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

const Token& Parser::consume(
    TokenType type,
    const std::string& message
) {
    if (check(type)) return advance();

    error(peek(), message);
    throw std::runtime_error(message);
}

void Parser::error(
    const Token& token,
    const std::string& message
) {
    errors_.push_back(
        token.location.file + ":" +
        std::to_string(token.location.line) + ":" +
        std::to_string(token.location.column) +
        ": error: " + message
    );
}

const std::vector<std::string>& Parser::errors() const {
    return errors_;
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        std::size_t oldPosition = current_;

        try {
            program->functions.push_back(parseFunction());
        }
        catch (const std::runtime_error&) {
            synchronize();
        }

        if (current_ == oldPosition && !isAtEnd()) {
            advance();
        }
    }

    return program;
}

Function Parser::parseFunction() {
    Function function;

    if (match(TokenType::KwInt)) {
        function.returnType = "int";
    }
    else if (match(TokenType::KwVoid)) {
        function.returnType = "void";
    }
    else {
        consume(TokenType::KwInt, "expected 'int' or 'void'");
    }

    function.name = consume(
        TokenType::Identifier,
        "expected function name"
    ).lexeme;

    consume(TokenType::LeftParen, "expected '(' after function name");

    if (!match(TokenType::KwVoid)) {
        if (!check(TokenType::RightParen)) {
            do {
                consume(TokenType::KwInt, "expected 'int' parameter");

                std::string parameter;

                if (check(TokenType::Identifier)) {
                    parameter = advance().lexeme;
                }

                function.parameters.push_back(parameter);

            } while (match(TokenType::Comma));
        }
    }

    consume(TokenType::RightParen, "expected ')' after parameters");

    if (match(TokenType::Semicolon)) {
        return function;
    }

    function.body = parseBlock();
    return function;
}

std::unique_ptr<Stmt> Parser::parseBlock() {
    consume(TokenType::LeftBrace, "expected '{'");

    auto block = std::make_unique<Stmt>();
    block->kind = StmtKind::Block;

    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        if (check(TokenType::KwInt)) {
            block->statements.push_back(parseDeclaration());
        }
        else {
            block->statements.push_back(parseStatement());
        }
    }

    consume(TokenType::RightBrace, "expected '}' after block");
    return block;
}

std::unique_ptr<Stmt> Parser::parseDeclaration() {
    consume(TokenType::KwInt, "expected 'int'");

    auto declaration = std::make_unique<Stmt>();
    declaration->kind = StmtKind::Declaration;

    declaration->name = consume(
        TokenType::Identifier,
        "expected variable name"
    ).lexeme;

    if (match(TokenType::Assign)) {
        declaration->expression = parseExpression();
    }

    consume(TokenType::Semicolon, "expected ';' after declaration");

    return declaration;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::LeftBrace)) {
        return parseBlock();
    }

    if (match(TokenType::KwIf)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::If;

        consume(TokenType::LeftParen, "expected '(' after if");
        stmt->expression = parseExpression();
        consume(TokenType::RightParen, "expected ')' after condition");

        stmt->statements.push_back(parseStatement());

        if (match(TokenType::KwElse)) {
            stmt->elseBranch = parseStatement();
        }

        return stmt;
    }

    if (match(TokenType::KwWhile)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::While;

        consume(TokenType::LeftParen, "expected '(' after while");
        stmt->expression = parseExpression();
        consume(TokenType::RightParen, "expected ')' after condition");

        stmt->statements.push_back(parseStatement());
        return stmt;
    }

    if (match(TokenType::KwFor)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::For;

        consume(TokenType::LeftParen, "expected '(' after for");

        auto header = std::make_unique<Expr>();
        header->kind = ExprKind::Binary;
        header->value = "for";

        if (match(TokenType::KwInt)) {
            std::string name = consume(
                TokenType::Identifier,
                "expected variable name in for"
            ).lexeme;

            auto init = std::make_unique<Expr>();
            init->kind = ExprKind::Assignment;
            init->value = name;

            if (match(TokenType::Assign)) {
                init->left = parseExpression();
            }

            header->left = std::move(init);
            consume(TokenType::Semicolon, "expected ';' after for initializer");
        }
        else if (!check(TokenType::Semicolon)) {
            header->left = parseExpression();
            consume(TokenType::Semicolon, "expected ';' after for initializer");
        }
        else {
            advance();
        }

        if (!check(TokenType::Semicolon)) {
            header->right = parseExpression();
        }

        consume(TokenType::Semicolon, "expected ';' after for condition");

        if (!check(TokenType::RightParen)) {
            header->arguments.push_back(parseExpression());
        }

        consume(TokenType::RightParen, "expected ')' after for clauses");

        stmt->expression = std::move(header);
        stmt->statements.push_back(parseStatement());

        return stmt;
    }

    if (match(TokenType::KwReturn)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Return;

        if (!check(TokenType::Semicolon)) {
            stmt->expression = parseExpression();
        }

        consume(TokenType::Semicolon, "expected ';' after return");
        return stmt;
    }

    if (match(TokenType::KwBreak)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Break;
        consume(TokenType::Semicolon, "expected ';' after break");
        return stmt;
    }

    if (match(TokenType::KwContinue)) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::Continue;
        consume(TokenType::Semicolon, "expected ';' after continue");
        return stmt;
    }

    auto stmt = std::make_unique<Stmt>();
    stmt->kind = StmtKind::Expression;

    if (!check(TokenType::Semicolon)) {
        stmt->expression = parseExpression();
    }

    consume(TokenType::Semicolon, "expected ';' after expression");
    return stmt;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto left = parseLogicalOr();

    if (match(TokenType::Assign)) {
        auto right = parseAssignment();

        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Assignment;
        expr->value = "=";
        expr->left = std::move(left);
        expr->right = std::move(right);

        return expr;
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();

    while (match(TokenType::OrOr)) {
        expr = makeBinary("||", std::move(expr), parseLogicalAnd());
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseEquality();

    while (match(TokenType::AndAnd)) {
        expr = makeBinary("&&", std::move(expr), parseEquality());
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseRelational();

    while (true) {
        if (match(TokenType::EqualEqual)) {
            expr = makeBinary("==", std::move(expr), parseRelational());
        }
        else if (match(TokenType::BangEqual)) {
            expr = makeBinary("!=", std::move(expr), parseRelational());
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseRelational() {
    auto expr = parseAdditive();

    while (true) {
        if (match(TokenType::Less)) {
            expr = makeBinary("<", std::move(expr), parseAdditive());
        }
        else if (match(TokenType::LessEqual)) {
            expr = makeBinary("<=", std::move(expr), parseAdditive());
        }
        else if (match(TokenType::Greater)) {
            expr = makeBinary(">", std::move(expr), parseAdditive());
        }
        else if (match(TokenType::GreaterEqual)) {
            expr = makeBinary(">=", std::move(expr), parseAdditive());
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (true) {
        if (match(TokenType::Plus)) {
            expr = makeBinary("+", std::move(expr), parseMultiplicative());
        }
        else if (match(TokenType::Minus)) {
            expr = makeBinary("-", std::move(expr), parseMultiplicative());
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto expr = parseUnary();

    while (true) {
        if (match(TokenType::Star)) {
            expr = makeBinary("*", std::move(expr), parseUnary());
        }
        else if (match(TokenType::Slash)) {
            expr = makeBinary("/", std::move(expr), parseUnary());
        }
        else if (match(TokenType::Percent)) {
            expr = makeBinary("%", std::move(expr), parseUnary());
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Plus)) {
        return makeBinary("unary+", nullptr, parseUnary());
    }

    if (match(TokenType::Minus)) {
        return makeBinary("unary-", nullptr, parseUnary());
    }

    if (match(TokenType::Bang)) {
        return makeBinary("!", nullptr, parseUnary());
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::IntegerLiteral)) {
        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Integer;
        expr->value = previous().lexeme;
        return expr;
    }

    if (match(TokenType::Identifier)) {
        std::string name = previous().lexeme;

        if (match(TokenType::LeftParen)) {
            auto expr = std::make_unique<Expr>();
            expr->kind = ExprKind::Call;
            expr->value = name;

            if (!check(TokenType::RightParen)) {
                do {
                    expr->arguments.push_back(parseExpression());
                } while (match(TokenType::Comma));
            }

            consume(TokenType::RightParen, "expected ')' after arguments");
            return expr;
        }

        auto expr = std::make_unique<Expr>();
        expr->kind = ExprKind::Variable;
        expr->value = name;
        return expr;
    }

    if (match(TokenType::LeftParen)) {
        auto expr = parseExpression();
        consume(TokenType::RightParen, "expected ')' after expression");
        return expr;
    }

    error(peek(), "expected an expression");
    throw std::runtime_error("invalid expression");
}

void Parser::synchronize() {
    if (!isAtEnd()) advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;

        switch (peek().type) {
            case TokenType::KwInt:
            case TokenType::KwVoid:
            case TokenType::KwIf:
            case TokenType::KwWhile:
            case TokenType::KwFor:
            case TokenType::KwReturn:
                return;
            default:
                advance();
                break;
        }
    }
}

}
