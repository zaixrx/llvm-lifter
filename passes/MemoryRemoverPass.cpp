#include "MemoryRemoverPass.hpp"
#include "RecoveryUtils.hpp"

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Analysis/Utils/Local.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

namespace {

bool removeMemory(llvm::Function& function, llvm::FunctionAnalysisManager& analyses) {
    if (function.isDeclaration() || !function.use_empty() ||
        function.hasFnAttribute("lifter.memory_removed")) return false;
    unsigned index = 1;
    if (auto attribute = function.getFnAttribute("lifter.memory"); attribute.isValid()) {
        if (attribute.getValueAsString().getAsInteger(10, index)) return false;
    } else if (!recovery::isLifted(function)) {
        return false;
    }
    if (index >= function.arg_size()) return false;
    auto* memory = function.getArg(index);
    auto* type = llvm::dyn_cast<llvm::PointerType>(memory->getType());
    const auto& layout = function.getDataLayout();
    if (!type || type->getAddressSpace() != 0 || layout.isNonIntegralPointerType(type) ||
        layout.getIndexSizeInBits(0) != layout.getPointerSizeInBits(0)) return false;

    llvm::SmallVector<llvm::Value*> worklist{memory};
    llvm::SmallVector<llvm::GetElementPtrInst*> geps;
    llvm::SmallPtrSet<llvm::Value*, 32> visited;
    while (!worklist.empty()) {
        auto* pointer = worklist.pop_back_val();
        if (!visited.insert(pointer).second) continue;
        for (auto* user : pointer->users()) {
            if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(user)) {
                if (gep->getPointerOperand() != pointer || gep->getType()->isVectorTy()) return false;
                geps.push_back(gep);
                worklist.push_back(gep);
            } else if (llvm::isa<llvm::BitCastInst>(user)) {
                worklist.push_back(user);
            } else if (auto* load = llvm::dyn_cast<llvm::LoadInst>(user)) {
                if (load->getPointerOperand() != pointer) return false;
            } else if (auto* store = llvm::dyn_cast<llvm::StoreInst>(user)) {
                if (store->getPointerOperand() != pointer || store->getValueOperand() == pointer) return false;
            } else {
                return false;
            }
        }
    }

    // Inbounds/nuw facts for the original buffer do not hold on a null base.
    for (auto* gep : geps) {
        llvm::IRBuilder<> builder(gep);
        auto* base = builder.CreatePtrToInt(gep->getPointerOperand(), layout.getIntPtrType(type));
        auto* offset = llvm::emitGEPOffset(&builder, layout, gep, true);
        auto* pointer = builder.CreateIntToPtr(builder.CreateAdd(base, offset), gep->getType());
        gep->replaceAllUsesWith(pointer);
        gep->eraseFromParent();
    }
    memory->replaceAllUsesWith(llvm::ConstantPointerNull::get(type));
    llvm::SmallVector<unsigned> kept;
    for (auto& argument : function.args()) {
        if (&argument != memory) kept.push_back(argument.getArgNo());
    }
    function.removeFnAttr(llvm::Attribute::Memory);
    for (auto& argument : function.args()) {
        if (argument.getType()->isPointerTy()) argument.removeAttr(llvm::Attribute::NoAlias);
    }
    function.removeFnAttr("lifter.memory");
    function.addFnAttr("lifter.memory_removed");
    auto* recovered = recovery::retainArguments(function, kept, analyses);
    recovery::simplify(*recovered, analyses);
    return true;
}

} // namespace

llvm::PreservedAnalyses MemoryRemoverPass::run(llvm::Module& module, llvm::ModuleAnalysisManager& analyses) {
    auto& functions = analyses.getResult<llvm::FunctionAnalysisManagerModuleProxy>(module).getManager();
    llvm::SmallVector<llvm::Function*> worklist;
    for (auto& function : module) {
        if (m_FunctionName.empty() || function.getName() == m_FunctionName) worklist.push_back(&function);
    }
    bool changed = false;
    for (auto* function : worklist) changed |= removeMemory(*function, functions);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
