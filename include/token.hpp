#ifndef CMINOR_TOKEN_HPP
#define CMINOR_TOKEN_HPP

#include <string>

namespace cminor {

enum class TokenType {
    EndOfFile,
    Invalid,

    Identifier,
    IntegerLiteral,

    KwInt,
    KwVoid,
    KwIf,
    KwElse,
    KwWhile,
    KwFor,
    KwBreak,
    KwContinue,
    KwReturn,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    EqualEqual,
    BangEqual,
    Bang,

    AndAnd,
    OrOr,

    Assign,

    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Semicolon
};

struct SourceLocation {
    std::string file;
    int line = 1;
    int column = 1;
};

struct Token {
    TokenType type;
    std::string lexeme;
    SourceLocation location;
};

const char* tokenTypeName(TokenType type);

} // namespace cminor

#endif
