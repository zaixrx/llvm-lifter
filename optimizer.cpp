#include "optimizer.hpp"
#include "passes/StateRemoverPass.hpp"
#include "passes/MemoryRemoverPass.hpp"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>

#include <llvm/IR/Module.h>
#include <utility>

Optimizer&& Optimizer::exhaustOptimizations(bool on) {
    EnableExhaustOptimizations = on;
    return std::move(*this);
}

Optimizer&& Optimizer::stateRemoverPass(bool on) {
    EnableStateRemoverPass = on;
    return std::move(*this);
}

Optimizer&& Optimizer::memoryRemoverPass(bool on) {
    EnableMemoryRemoverPass = on;
    return std::move(*this);
}

llvm::Function* Optimizer::run() {
    llvm::PassBuilder PB;
    llvm::LoopAnalysisManager LAM;
    llvm::CGSCCAnalysisManager CAM;
    llvm::ModuleAnalysisManager MAM;
    llvm::FunctionAnalysisManager FAM;

    const auto level = EnableExhaustOptimizations ? llvm::OptimizationLevel::O3 : llvm::OptimizationLevel::O1;
    auto FPM = PB.buildFunctionSimplificationPipeline(level, llvm::ThinOrFullLTOPhase::ThinLTOPreLink);

    PB.registerLoopAnalyses(LAM);
    PB.registerCGSCCAnalyses(CAM);
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.crossRegisterProxies(LAM, FAM, CAM, MAM);

    auto* module = m_Function->getParent();
    const auto name = m_Function->getName().str();
    llvm::ModulePassManager recovery;
    if (EnableStateRemoverPass) recovery.addPass(StateRemoverPass(name));
    if (EnableMemoryRemoverPass) recovery.addPass(MemoryRemoverPass(name));
    recovery.run(*module, MAM);
    auto* function = module->getFunction(name);
    FPM.run(*function, FAM);
    m_Function = function;
    return function;
}
