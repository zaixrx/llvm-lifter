#pragma once

#include <llvm/IR/PassManager.h>
#include <string>
#include <utility>

class StateRemoverPass : public llvm::PassInfoMixin<StateRemoverPass>
{
public:
    // Signature recovery must run after lifting is complete.
    explicit StateRemoverPass(std::string functionName = {}) : m_FunctionName(std::move(functionName)) {}
    llvm::PreservedAnalyses run(llvm::Module& module, llvm::ModuleAnalysisManager& analyses);

private:
    std::string m_FunctionName;
};

