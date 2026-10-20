#pragma once

#include <llvm/IR/PassManager.h>
#include <string>
#include <utility>

class MemoryRemoverPass : public llvm::PassInfoMixin<MemoryRemoverPass> {
public:
    // Guest addresses must already refer to native process memory.
    explicit MemoryRemoverPass(std::string functionName = {}) : m_FunctionName(std::move(functionName)) {}
    llvm::PreservedAnalyses run(llvm::Module& module, llvm::ModuleAnalysisManager& analyses);

private:
    std::string m_FunctionName;
};
