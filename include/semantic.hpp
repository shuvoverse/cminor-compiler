#ifndef CMINOR_SEMANTIC_HPP
#define CMINOR_SEMANTIC_HPP

#include "ast.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cminor {

class SemanticAnalyzer {
public:
    void analyze(const Program& program);

    const std::vector<std::string>& errors() const;

private:
    std::vector<std::string> errors_;

    std::vector<std::unordered_set<std::string>> scopes_;

    std::unordered_map<std::string, std::string> functions_;

    int loopDepth_ = 0;

    void error(const std::string& message);

    void enterScope();
    void leaveScope();

    bool declareVariable(const std::string& name);
    bool variableExists(const std::string& name) const;

    void analyzeFunction(const Function& function);
    void analyzeStatement(const Stmt& statement);
    void analyzeExpression(const Expr& expression);
};

}

#endif
