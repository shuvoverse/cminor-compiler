#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void showHelp() {
    std::cout
        << "C Minor Compiler v1.0.0\n"
        << "Usage: cminorcc [options] input.cm\n\n"
        << "Options:\n"
        << "  --help          Show help\n"
        << "  --version       Show version\n"
        << "  --check         Check source code\n"
        << "  --dump-tokens   Display tokens\n";
}

int main(int argc, char* argv[]) {
    bool dumpTokens = false;
    bool checkOnly = false;
    std::string inputFile;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            showHelp();
            return 0;
        }

        if (arg == "--version") {
            std::cout << "C Minor Compiler version 1.0.0\n";
            return 0;
        }

        if (arg == "--dump-tokens") {
            dumpTokens = true;
            continue;
        }

        if (arg == "--check") {
            checkOnly = true;
            continue;
        }

        if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << '\n';
            return 2;
        }

        if (inputFile.empty()) {
            inputFile = arg;
        } else {
            std::cerr << "Multiple input files are not supported.\n";
            return 2;
        }
    }

    if (inputFile.empty()) {
        showHelp();
        return 2;
    }

    std::ifstream file(inputFile);

    if (!file) {
        std::cerr << "Cannot open file: " << inputFile << '\n';
        return 2;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    cminor::Lexer lexer(buffer.str(), inputFile);
    std::vector<cminor::Token> tokens = lexer.tokenize();

    if (!lexer.errors().empty()) {
        for (const auto& error : lexer.errors()) {
            std::cerr << error << '\n';
        }
        return 1;
    }

    if (dumpTokens) {
        for (const auto& token : tokens) {
            std::cout << token.location.line << ":"
                      << token.location.column << " "
                      << cminor::tokenTypeName(token.type)
                      << " " << token.lexeme << '\n';
        }
    }

    cminor::Parser parser(tokens);
    auto program = parser.parse();

    if (!parser.errors().empty()) {
        for (const auto& error : parser.errors()) {
            std::cerr << error << '\n';
        }
        return 1;
    }

    cminor::SemanticAnalyzer semantic;
    semantic.analyze(*program);

    if (!semantic.errors().empty()) {
        for (const auto& error : semantic.errors()) {
            std::cerr << error << '\n';
        }
        return 1;
    }

    if (checkOnly) {
        std::cout << "Lexical analysis: successful\n";
        std::cout << "Syntax analysis: successful\n";
        std::cout << "Semantic analysis: successful\n";
    }

    return 0;
}
