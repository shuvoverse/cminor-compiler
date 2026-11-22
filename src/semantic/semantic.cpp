#include "semantic.hpp"

namespace cminor {

void SemanticAnalyzer::error(const std::string& message) {
    errors_.push_back("Semantic error: " + message);
}

const std::vector<std::string>& SemanticAnalyzer::errors() const {
    return errors_;
}

void SemanticAnalyzer::enterScope() {
    scopes_.push_back({});
}

void SemanticAnalyzer::leaveScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool SemanticAnalyzer::declareVariable(const std::string& name) {
    if (scopes_.empty()) {
        enterScope();
    }

    auto& currentScope = scopes_.back();

    if (currentScope.count(name)) {
        error("duplicate variable declaration: " + name);
        return false;
    }

    currentScope.insert(name);
    return true;
}

bool SemanticAnalyzer::variableExists(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->count(name)) {
            return true;
        }
    }

    return false;
}

void SemanticAnalyzer::analyzeExpression(const Expr& expression) {
    if (expression.kind == ExprKind::Variable) {
        if (!variableExists(expression.value)) {
            error("use of undeclared variable: " + expression.value);
        }
        return;
    }

    if (expression.kind == ExprKind::Assignment) {
        if (expression.left) {
            if (expression.left->kind != ExprKind::Variable) {
                error("assignment requires a variable on the left side");
            } else if (!variableExists(expression.left->value)) {
                error("assignment to undeclared variable: " +
                      expression.left->value);
            }
        }

        if (expression.right) {
            analyzeExpression(*expression.right);
        }

        if (expression.left &&
            expression.left->kind != ExprKind::Variable) {
            analyzeExpression(*expression.left);
        }

        return;
    }

    if (expression.left) {
        analyzeExpression(*expression.left);
    }

    if (expression.right) {
        analyzeExpression(*expression.right);
    }

    for (const auto& argument : expression.arguments) {
        if (argument) {
            analyzeExpression(*argument);
        }
    }
}

void SemanticAnalyzer::analyzeStatement(const Stmt& statement) {
    switch (statement.kind) {
        case StmtKind::Block: {
            enterScope();

            for (const auto& child : statement.statements) {
                if (child) {
                    analyzeStatement(*child);
                }
            }

            leaveScope();
            break;
        }

        case StmtKind::Declaration:
            declareVariable(statement.name);

            if (statement.expression) {
                analyzeExpression(*statement.expression);
            }
            break;

        case StmtKind::Expression:
        case StmtKind::Return:
            if (statement.expression) {
                analyzeExpression(*statement.expression);
            }
            break;

        case StmtKind::If:
            if (statement.expression) {
                analyzeExpression(*statement.expression);
            }

            for (const auto& child : statement.statements) {
                if (child) {
                    analyzeStatement(*child);
                }
            }

            if (statement.elseBranch) {
                analyzeStatement(*statement.elseBranch);
            }
            break;

        case StmtKind::While:
            if (statement.expression) {
                analyzeExpression(*statement.expression);
            }

            ++loopDepth_;

            for (const auto& child : statement.statements) {
                if (child) {
                    analyzeStatement(*child);
                }
            }

            --loopDepth_;
            break;

        case StmtKind::For:
            enterScope();

            if (statement.expression) {
                analyzeExpression(*statement.expression);
            }

            ++loopDepth_;

            for (const auto& child : statement.statements) {
                if (child) {
                    analyzeStatement(*child);
                }
            }

            --loopDepth_;
            leaveScope();
            break;

        case StmtKind::Break:
            if (loopDepth_ == 0) {
                error("'break' used outside a loop");
            }
            break;

        case StmtKind::Continue:
            if (loopDepth_ == 0) {
                error("'continue' used outside a loop");
            }
            break;
    }
}

void SemanticAnalyzer::analyzeFunction(const Function& function) {
    scopes_.clear();
    enterScope();

    for (const auto& parameter : function.parameters) {
        if (!parameter.empty()) {
            declareVariable(parameter);
        }
    }

    if (function.body) {
        analyzeStatement(*function.body);
    }

    scopes_.clear();
}

void SemanticAnalyzer::analyze(const Program& program) {
    errors_.clear();
    functions_.clear();

    for (const auto& function : program.functions) {
        auto result = functions_.emplace(
            function.name,
            function.returnType
        );

        if (!result.second &&
            result.first->second != function.returnType) {
            error("conflicting return type for function: " + function.name);
        }
    }

    for (const auto& function : program.functions) {
        if (function.name == "main") {
            if (function.returnType != "int" ||
                !function.parameters.empty()) {
                error("main must have signature int main(void)");
            }
        }

        analyzeFunction(function);
    }

    if (functions_.find("main") == functions_.end()) {
        error("program must define int main(void)");
    }
}

}
