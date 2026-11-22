#include "lexer.hpp"

#include <cctype>
#include <limits>
#include <unordered_map>

namespace cminor {

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename) {}

char Lexer::current() const {
    return isAtEnd() ? '\0' : source_[position_];
}

char Lexer::peek(std::size_t offset) const {
    if (position_ + offset >= source_.size()) return '\0';
    return source_[position_ + offset];
}

bool Lexer::isAtEnd() const {
    return position_ >= source_.size();
}

char Lexer::advance() {
    char c = current();

    if (!isAtEnd()) {
        ++position_;

        if (c == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
    }

    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || current() != expected) return false;
    advance();
    return true;
}

void Lexer::addError(const std::string& message, int line, int column) {
    errors_.push_back(
        filename_ + ":" + std::to_string(line) + ":" +
        std::to_string(column) + ": error: " + message
    );
}

Token Lexer::makeToken(
    TokenType type,
    std::size_t start,
    int startLine,
    int startColumn
) const {
    return {
        type,
        source_.substr(start, position_ - start),
        {filename_, startLine, startColumn}
    };
}

const std::vector<std::string>& Lexer::errors() const {
    return errors_;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        if (std::isspace(static_cast<unsigned char>(current()))) {
            advance();
            continue;
        }

        if (current() == '/' && peek() == '/') {
            while (!isAtEnd() && current() != '\n') advance();
            continue;
        }

        if (current() == '/' && peek() == '*') {
            int commentLine = line_;
            int commentColumn = column_;

            advance();
            advance();

            bool closed = false;

            while (!isAtEnd()) {
                if (current() == '*' && peek() == '/') {
                    advance();
                    advance();
                    closed = true;
                    break;
                }
                advance();
            }

            if (!closed) {
                addError("unterminated block comment", commentLine, commentColumn);
                return;
            }

            continue;
        }

        break;
    }
}

Token Lexer::scanIdentifierOrKeyword() {
    std::size_t start = position_;
    int startLine = line_;
    int startColumn = column_;

    while (true) {
        unsigned char c = static_cast<unsigned char>(current());

        bool letter = (c >= 'a' && c <= 'z') ||
                      (c >= 'A' && c <= 'Z');

        bool digit = c >= '0' && c <= '9';

        if (!letter && !digit && c != '_') break;

        advance();
    }

    std::string text = source_.substr(start, position_ - start);

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KwInt},
        {"void", TokenType::KwVoid},
        {"if", TokenType::KwIf},
        {"else", TokenType::KwElse},
        {"while", TokenType::KwWhile},
        {"for", TokenType::KwFor},
        {"break", TokenType::KwBreak},
        {"continue", TokenType::KwContinue},
        {"return", TokenType::KwReturn}
    };

    auto it = keywords.find(text);

    if (it != keywords.end()) {
        return makeToken(it->second, start, startLine, startColumn);
    }

    return makeToken(TokenType::Identifier, start, startLine, startColumn);
}

Token Lexer::scanNumber() {
    std::size_t start = position_;
    int startLine = line_;
    int startColumn = column_;

    while (std::isdigit(static_cast<unsigned char>(current()))) {
        advance();
    }

    if ((current() >= 'a' && current() <= 'z') ||
        (current() >= 'A' && current() <= 'Z') ||
        current() == '_') {

        while (std::isalnum(static_cast<unsigned char>(current())) ||
               current() == '_') {
            advance();
        }

        addError("invalid integer literal", startLine, startColumn);
        return makeToken(TokenType::Invalid, start, startLine, startColumn);
    }

    std::string text = source_.substr(start, position_ - start);

    if (text.size() > 1 && text[0] == '0') {
        addError("leading zero in integer literal", startLine, startColumn);
        return makeToken(TokenType::Invalid, start, startLine, startColumn);
    }

    try {
        unsigned long long value = std::stoull(text);

        if (value > 2147483647ULL) {
            addError(
                "integer literal exceeds maximum signed 32-bit value",
                startLine,
                startColumn
            );
            return makeToken(TokenType::Invalid, start, startLine, startColumn);
        }
    } catch (...) {
        addError("invalid integer literal", startLine, startColumn);
        return makeToken(TokenType::Invalid, start, startLine, startColumn);
    }

    return makeToken(TokenType::IntegerLiteral, start, startLine, startColumn);
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();

    std::size_t start = position_;
    int startLine = line_;
    int startColumn = column_;

    if (isAtEnd()) {
        return makeToken(TokenType::EndOfFile, start, startLine, startColumn);
    }

    unsigned char first = static_cast<unsigned char>(current());

    bool letter = (first >= 'a' && first <= 'z') ||
                  (first >= 'A' && first <= 'Z');

    if (letter || first == '_') {
        return scanIdentifierOrKeyword();
    }

    if (first >= '0' && first <= '9') {
        return scanNumber();
    }

    char c = advance();

    switch (c) {
        case '+': return makeToken(TokenType::Plus, start, startLine, startColumn);
        case '-': return makeToken(TokenType::Minus, start, startLine, startColumn);
        case '*': return makeToken(TokenType::Star, start, startLine, startColumn);
        case '/': return makeToken(TokenType::Slash, start, startLine, startColumn);
        case '%': return makeToken(TokenType::Percent, start, startLine, startColumn);

        case '(': return makeToken(TokenType::LeftParen, start, startLine, startColumn);
        case ')': return makeToken(TokenType::RightParen, start, startLine, startColumn);
        case '{': return makeToken(TokenType::LeftBrace, start, startLine, startColumn);
        case '}': return makeToken(TokenType::RightBrace, start, startLine, startColumn);
        case ',': return makeToken(TokenType::Comma, start, startLine, startColumn);
        case ';': return makeToken(TokenType::Semicolon, start, startLine, startColumn);

        case '=':
            return makeToken(match('=') ? TokenType::EqualEqual : TokenType::Assign,
                             start, startLine, startColumn);

        case '!':
            return makeToken(match('=') ? TokenType::BangEqual : TokenType::Bang,
                             start, startLine, startColumn);

        case '<':
            return makeToken(match('=') ? TokenType::LessEqual : TokenType::Less,
                             start, startLine, startColumn);

        case '>':
            return makeToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater,
                             start, startLine, startColumn);

        case '&':
            if (match('&'))
                return makeToken(TokenType::AndAnd, start, startLine, startColumn);
            break;

        case '|':
            if (match('|'))
                return makeToken(TokenType::OrOr, start, startLine, startColumn);
            break;
    }

    addError("invalid character", startLine, startColumn);
    return makeToken(TokenType::Invalid, start, startLine, startColumn);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token = scanToken();
        tokens.push_back(token);

        if (token.type == TokenType::EndOfFile) break;
    }

    return tokens;
}

} // namespace cminor
