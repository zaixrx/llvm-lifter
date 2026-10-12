#include "optimizer.hpp"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>

#include <algorithm>

Optimizer&& Optimizer::exhaustOptimizations(bool on) {
    ExhaustOptimizations = on;
    return std::move(*this);
}

void Optimizer::run() const {
    llvm::PassBuilder PB;
    llvm::LoopAnalysisManager LAM;
    llvm::CGSCCAnalysisManager CAM;
    llvm::ModuleAnalysisManager MAM;
    llvm::FunctionAnalysisManager FAM;

    auto FPM = PB.buildFunctionSimplificationPipeline(llvm::OptimizationLevel::O3, llvm::ThinOrFullLTOPhase::ThinLTOPreLink);

    PB.registerLoopAnalyses(LAM);
    PB.registerCGSCCAnalyses(CAM);
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.crossRegisterProxies(LAM, FAM, CAM, MAM);

    FPM.run(*m_Function, FAM);
}
