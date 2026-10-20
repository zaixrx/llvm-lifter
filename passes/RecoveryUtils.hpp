#pragma once

#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/PassManager.h>

namespace recovery {
bool isLifted(const llvm::Function& function);
void simplify(llvm::Function& function, llvm::FunctionAnalysisManager& analyses);
llvm::Function* retainArguments(llvm::Function& function, llvm::ArrayRef<unsigned> kept,
                                llvm::FunctionAnalysisManager& analyses);
} // namespace recovery
