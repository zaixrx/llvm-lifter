#include "RecoveryUtils.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/ADCE.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Cloning.h>

bool recovery::isLifted(const llvm::Function& function) {
    return !function.isDeclaration() && function.getName().starts_with("lifted_") &&
        function.getReturnType()->isVoidTy() && !function.isVarArg() &&
        function.arg_size() == 2 && function.getArg(0)->getType()->isPointerTy() &&
        function.getArg(1)->getType()->isPointerTy() &&
        function.getCallingConv() == llvm::CallingConv::C;
}

void recovery::simplify(llvm::Function& function, llvm::FunctionAnalysisManager& analyses) {
    llvm::FunctionPassManager passes;
    passes.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
    passes.addPass(llvm::InstCombinePass());
    passes.addPass(llvm::SimplifyCFGPass());
    passes.addPass(llvm::ADCEPass());
    passes.run(function, analyses);
}

llvm::Function* recovery::retainArguments(llvm::Function& function, llvm::ArrayRef<unsigned> kept,
                                         llvm::FunctionAnalysisManager& analyses) {
    llvm::SmallVector<llvm::Type*> types;
    llvm::SmallVector<llvm::AttributeSet> attributes;
    for (unsigned index : kept) {
        types.push_back(function.getArg(index)->getType());
        attributes.push_back(function.getAttributes().getParamAttrs(index));
    }
    auto* recovered = llvm::Function::Create(
        llvm::FunctionType::get(function.getReturnType(), types, false), function.getLinkage(),
        function.getAddressSpace(), "", function.getParent());
    llvm::ValueToValueMapTy values;
    for (auto& argument : function.args()) {
        assert(argument.use_empty() || llvm::is_contained(kept, argument.getArgNo()));
        values[&argument] = llvm::PoisonValue::get(argument.getType());
    }
    for (unsigned index = 0; index < kept.size(); ++index) {
        auto* old = function.getArg(kept[index]);
        auto* argument = recovered->getArg(index);
        argument->setName(old->getName());
        values[old] = argument;
    }
    llvm::SmallVector<llvm::ReturnInst*> returns;
    llvm::CloneFunctionInto(recovered, &function, values,
        llvm::CloneFunctionChangeType::LocalChangesOnly, returns);
    recovered->setAttributes(llvm::AttributeList::get(function.getContext(),
        function.getAttributes().getFnAttrs(), function.getAttributes().getRetAttrs(), attributes));
    recovered->takeName(&function);
    analyses.clear(function, function.getName());
    function.eraseFromParent();
    return recovered;
}
