#include "StateRemoverPass.hpp"
#include "RecoveryUtils.hpp"
#include "../semantics.hpp"

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <array>

namespace {

struct Field {
    const char* name;
    std::size_t offset;
    unsigned bits;
};

// Keep the System V integer argument registers first and in ABI order.
constexpr std::array fields {
    Field{"rdi", offsetof(lifter::X86State, rdi), 64},
    Field{"rsi", offsetof(lifter::X86State, rsi), 64},
    Field{"rdx", offsetof(lifter::X86State, rdx), 64},
    Field{"rcx", offsetof(lifter::X86State, rcx), 64},
    Field{"r8", offsetof(lifter::X86State, r8), 64},
    Field{"r9", offsetof(lifter::X86State, r9), 64},
    Field{"rax", offsetof(lifter::X86State, rax), 64},
    Field{"rbx", offsetof(lifter::X86State, rbx), 64},
    Field{"rbp", offsetof(lifter::X86State, rbp), 64},
    Field{"rsp", offsetof(lifter::X86State, rsp), 64},
    Field{"r10", offsetof(lifter::X86State, r10), 64},
    Field{"r11", offsetof(lifter::X86State, r11), 64},
    Field{"r12", offsetof(lifter::X86State, r12), 64},
    Field{"r13", offsetof(lifter::X86State, r13), 64},
    Field{"r14", offsetof(lifter::X86State, r14), 64},
    Field{"r15", offsetof(lifter::X86State, r15), 64},
    Field{"rip", offsetof(lifter::X86State, rip), 64},
    Field{"gs", offsetof(lifter::X86State, gs), 64},
    Field{"cf", offsetof(lifter::X86State, cf), 8},
    Field{"pf", offsetof(lifter::X86State, pf), 8},
    Field{"af", offsetof(lifter::X86State, af), 8},
    Field{"zf", offsetof(lifter::X86State, zf), 8},
    Field{"sf", offsetof(lifter::X86State, sf), 8},
    Field{"of", offsetof(lifter::X86State, of), 8}
};

bool canRecover(llvm::Function& function) {
    if (!recovery::isLifted(function) || !function.use_empty() ||
        function.hasFnAttribute("lifter.state_removed") ||
        function.hasFnAttribute("lifter.memory_removed")) return false;

    const auto& layout = function.getDataLayout();
    if (!layout.isLittleEndian() || layout.getPointerSizeInBits(0) != 64 ||
        function.getArg(0)->getType()->getPointerAddressSpace() != 0 ||
        function.getArg(1)->getType()->getPointerAddressSpace() != 0) return false;
    auto* state = function.getArg(0);
    llvm::SmallVector<llvm::Value*> worklist{state};
    llvm::SmallPtrSet<llvm::Value*, 32> visited;
    while (!worklist.empty()) {
        auto* pointer = worklist.pop_back_val();
        if (!visited.insert(pointer).second) continue;
        for (auto* user : pointer->users()) {
            if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(user)) {
                if (!gep->hasAllConstantIndices()) return false;
                worklist.push_back(gep);
                continue;
            }
            if (llvm::isa<llvm::BitCastInst>(user)) {
                worklist.push_back(user);
                continue;
            }
            llvm::Type* type = nullptr;
            if (auto* load = llvm::dyn_cast<llvm::LoadInst>(user)) {
                if (!load->isSimple() || load->getPointerOperand() != pointer) return false;
                type = load->getType();
            } else if (auto* store = llvm::dyn_cast<llvm::StoreInst>(user)) {
                if (!store->isSimple() || store->getPointerOperand() != pointer ||
                    store->getValueOperand() == pointer) return false;
                type = store->getValueOperand()->getType();
            } else {
                return false;
            }
            if (!type->isIntegerTy()) return false;
            int64_t offset = 0;
            if (llvm::GetPointerBaseWithConstantOffset(pointer, offset, layout) != state ||
                offset < 0 || offset > static_cast<int64_t>(offsetof(lifter::X86State, of)) ||
                layout.getTypeStoreSize(type) > offsetof(lifter::X86State, of) + 1 - offset) {
                return false;
            }
        }
    }

    for (auto& instruction : llvm::instructions(function)) {
        if (auto* call = llvm::dyn_cast<llvm::CallBase>(&instruction)) {
            auto* callee = call->getCalledFunction();
            if (callee && (callee->getName() == "__lifter_jmp_handler" ||
                           callee->getName() == "__lifter_call_handler")) return false;
        }
        // Placeholder returns at undecoded destinations are not native RETs.
        if (auto* ret = llvm::dyn_cast<llvm::ReturnInst>(&instruction)) {
            auto* call = llvm::dyn_cast_or_null<llvm::CallInst>(ret->getPrevNode());
            if (!call || !call->getCalledFunction() ||
                call->getCalledFunction()->getName() != "__lifter_ret_handler" ||
                !call->getType()->isVoidTy()) return false;
        }
    }
    return true;
}

llvm::Function* recover(llvm::Function& function, llvm::FunctionAnalysisManager& analyses) {
    if (!canRecover(function)) return nullptr;

    auto& context = function.getContext();
    auto* i64 = llvm::Type::getInt64Ty(context);
    llvm::SmallVector<llvm::Type*> types;
    for (const auto& field : fields) types.push_back(llvm::IntegerType::get(context, field.bits));
    types.push_back(function.getArg(1)->getType());
    auto* candidate = llvm::Function::Create(llvm::FunctionType::get(i64, types, false),
        function.getLinkage(), function.getAddressSpace(), "", function.getParent());
    auto* entry = llvm::BasicBlock::Create(context, "initialize_state", candidate);
    llvm::IRBuilder<> builder(entry);
    auto* state = builder.CreateAlloca(
        llvm::ArrayType::get(builder.getInt8Ty(), sizeof(lifter::X86State)), nullptr, "state.local");
    state->setAlignment(llvm::Align(alignof(lifter::X86State)));
    for (unsigned index = 0; index < fields.size(); ++index) {
        const auto& field = fields[index];
        auto* argument = candidate->getArg(index);
        argument->setName(field.name);
        auto* pointer = builder.CreateGEP(builder.getInt8Ty(), state, builder.getInt64(field.offset));
        builder.CreateStore(argument, pointer);
    }
    auto* memory = candidate->getArg(fields.size());
    memory->setName("memory");
    llvm::ValueToValueMapTy values;
    values[function.getArg(0)] = state;
    values[function.getArg(1)] = memory;
    llvm::SmallVector<llvm::ReturnInst*> returns;
    llvm::CloneFunctionInto(candidate, &function, values,
        llvm::CloneFunctionChangeType::LocalChangesOnly, returns);
    // Cloning copies parameter attributes, which no longer match the new types.
    candidate->setAttributes(llvm::AttributeList{});
    candidate->addParamAttr(memory->getArgNo(), llvm::Attribute::NoAlias);
    candidate->addFnAttr("lifter.state_removed");
    builder.CreateBr(llvm::cast<llvm::BasicBlock>(values[&function.getEntryBlock()]));
    for (auto* ret : returns) {
        llvm::cast<llvm::CallInst>(ret->getPrevNode())->eraseFromParent();
        builder.SetInsertPoint(ret);
        auto* result = builder.CreateLoad(i64, state, "result");
        builder.CreateRet(result);
        ret->eraseFromParent();
    }
    recovery::simplify(*candidate, analyses);

    unsigned abiCount = 0;
    for (unsigned index = 0; index < 6; ++index) {
        if (!candidate->getArg(index)->use_empty()) abiCount = index + 1;
    }
    llvm::SmallVector<unsigned> kept;
    for (unsigned index = 0; index < fields.size(); ++index) {
        // Preserve positions through the last live System V argument. Keep
        // other live machine inputs explicit instead of making them undef.
        if (index < abiCount || !candidate->getArg(index)->use_empty()) kept.push_back(index);
    }
    kept.push_back(memory->getArgNo());
    auto* recovered = recovery::retainArguments(*candidate, kept, analyses);
    recovered->addFnAttr("lifter.memory", std::to_string(kept.size() - 1));
    recovered->takeName(&function);
    analyses.clear(function, function.getName());
    function.eraseFromParent();
    return recovered;
}

} // namespace

llvm::PreservedAnalyses StateRemoverPass::run(llvm::Module& module, llvm::ModuleAnalysisManager& analyses) {
    auto& functions = analyses.getResult<llvm::FunctionAnalysisManagerModuleProxy>(module).getManager();
    llvm::SmallVector<llvm::Function*> worklist;
    for (auto& function : module) {
        if (m_FunctionName.empty() || function.getName() == m_FunctionName) worklist.push_back(&function);
    }
    bool changed = false;
    for (auto* function : worklist) changed |= recover(*function, functions) != nullptr;
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
