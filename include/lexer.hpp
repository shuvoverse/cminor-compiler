#ifndef CMINOR_LEXER_HPP
#define CMINOR_LEXER_HPP

#include "token.hpp"

#include <string>
#include <vector>

namespace cminor {

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename);

    std::vector<Token> tokenize();

    const std::vector<std::string>& errors() const;

private:
    std::string source_;
    std::string filename_;

    std::size_t position_ = 0;
    int line_ = 1;
    int column_ = 1;

    std::vector<std::string> errors_;

    char current() const;
    char peek(std::size_t offset = 1) const;
    bool isAtEnd() const;

    char advance();
    bool match(char expected);

    void addError(const std::string& message, int line, int column);

    void skipWhitespaceAndComments();

    Token makeToken(
        TokenType type,
        std::size_t start,
        int startLine,
        int startColumn
    ) const;

    Token scanToken();
    Token scanIdentifierOrKeyword();
    Token scanNumber();

};

} // namespace cminor

#endif
