#include "token.hpp"

namespace cminor {

const char* tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::EndOfFile: return "EOF";
        case TokenType::Invalid: return "INVALID";

        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::IntegerLiteral: return "INTEGER";

        case TokenType::KwInt: return "KW_INT";
        case TokenType::KwVoid: return "KW_VOID";
        case TokenType::KwIf: return "KW_IF";
        case TokenType::KwElse: return "KW_ELSE";
        case TokenType::KwWhile: return "KW_WHILE";
        case TokenType::KwFor: return "KW_FOR";
        case TokenType::KwBreak: return "KW_BREAK";
        case TokenType::KwContinue: return "KW_CONTINUE";
        case TokenType::KwReturn: return "KW_RETURN";

        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";

        case TokenType::Less: return "LESS";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";

        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Bang: return "BANG";

        case TokenType::AndAnd: return "AND_AND";
        case TokenType::OrOr: return "OR_OR";

        case TokenType::Assign: return "ASSIGN";

        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::Comma: return "COMMA";
        case TokenType::Semicolon: return "SEMICOLON";
    }

    return "UNKNOWN";
}

} // namespace cminor
