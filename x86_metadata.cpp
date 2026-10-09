#include "semantics.hpp"

#include <format>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/CodeGen.h>

namespace lifter {

using namespace llvm;

// The module/context exist, but Semantics::createTypes() has not run yet.
Semantics_x86::Semantics_x86(Semantics *sem) {
    auto* i64 = Type::getInt64Ty(sem->m_Context);
    auto* i1 = Type::getInt1Ty(sem->m_Context);

    auto createFunc = [&sem](const char *name, llvm::Type *result, llvm::ArrayRef<llvm::Type*> params) -> llvm::Function* {
        auto* func = sem->m_Module.getFunction(name);
        auto* type = llvm::FunctionType::get(result, params, false);
        if (func && func->getFunctionType() != type) {
            throw std::invalid_argument(std::format("module defined {} has incompatible type", name));
        }
        if (!func) {
            func = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, sem->m_Module);
        }
        return func;
    };

    jumpHandler = createFunc("__lifter_jmp_handler", Type::getVoidTy(sem->m_Context), {i64});
    callHandler = createFunc("__lifter_call_handler", Type::getVoidTy(sem->m_Context), {i64});
    retHandler = createFunc("__lifter_ret_handler", Type::getVoidTy(sem->m_Context), {i64});
    _undefFlags.emplace(AF, createFunc("__lifter_undef_af", i1, {i64}));
}

// data
auto Semantics_x86::HandleMov(Semantics* sem) -> std::vector<Successor> {
    auto value = sem->opRead(1);
    sem->opWrite(0, value);
    return {};
}

// arith
auto Semantics_x86::HelperUpdateFlags(Semantics* sem, Value *value) -> void {
    auto resultIsZero = [&](Value *value) -> Value* {
        return sem->m_Builder.CreateICmpEQ(value, ConstantInt::get(value->getType(), 0));
    };
    auto resultSignBit = [&](Value *value) -> Value* {
        const auto width = value->getType()->getIntegerBitWidth();
        value = sem->m_Builder.CreateBinOp(Instruction::BinaryOps::LShr, value, ConstantInt::get(value->getType(), width - 1));
        return sem->m_Builder.CreateICmpEQ(value, ConstantInt::get(value->getType(), 1));
    };
    auto resultParityEven = [&](Value *value) -> Value* {
        Value *x, *y;
        x = sem->m_Builder.CreateZExtOrTrunc(value, sem->_i8Ty);
        for (unsigned shift : {4u, 2u, 1u}) {
            y = sem->m_Builder.CreateBinOp(Instruction::BinaryOps::LShr, x, ConstantInt::get(sem->_i8Ty, shift));
            x = sem->m_Builder.CreateBinOp(Instruction::BinaryOps::Xor, x, y);
        }
        x = sem->m_Builder.CreateAnd(x, ConstantInt::get(sem->_i8Ty, 1));
        return sem->m_Builder.CreateICmpEQ(x, ConstantInt::get(sem->_i8Ty, 0));
    };

    sem->flagWrite(Flag::PF, resultParityEven(value));
    sem->flagWrite(Flag::ZF, resultIsZero(value));
    sem->flagWrite(Flag::SF, resultSignBit(value));
    sem->flagWrite(Flag::CF, sem->m_Builder.getInt1(false));
    sem->flagWrite(Flag::OF, sem->m_Builder.getInt1(false));
    sem->flagWriteUndef(Flag::AF);
}

auto Semantics_x86::HelperBinaryOP(Semantics* sem, llvm::Instruction::BinaryOps Opc, void(Semantics_x86::*helper)(Semantics*, llvm::Value*)) -> std::vector<Successor> {
    auto lhs = sem->opRead(0);
    auto rhs = sem->m_Builder.CreateZExtOrTrunc(sem->opRead(1), lhs->getType());
    auto res = sem->m_Builder.CreateBinOp(Opc, lhs, rhs);
    sem->opWrite(0, res);
    (this->*helper)(sem, res);
    return {};
}

auto Semantics_x86::HandleAnd(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::And, &Semantics_x86::HelperUpdateFlags);
}

auto Semantics_x86::HandleOr(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::Or, &Semantics_x86::HelperUpdateFlags);
}

auto Semantics_x86::HandleXor(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::Xor, &Semantics_x86::HelperUpdateFlags);
}

auto Semantics_x86::HelperConditinalJump(Semantics *sem, Value *cond) -> std::vector<Successor> {
    uintptr_t falseAddr = sem->m_Insn->address + sem->m_Insn->size;
    uintptr_t trueAddr = sem->m_Insn->detail->x86.operands[0].imm;

    sem->m_Builder.CreateCondBr(cond, sem->getOrCreateBlock(trueAddr), sem->getOrCreateBlock(falseAddr));

    auto src = sem->m_Insn->address;
    return { 
        Successor(src, sem->m_Builder.getInt64(falseAddr)),
        Successor(src, sem->m_Builder.getInt64(trueAddr)),
    };
}

auto Semantics_x86::HelperCCCond(Semantics *sem, CCCond cc) {
    auto cf = [&]() -> Value* {
        return sem->flagRead(Flag::CF);
    };

    auto zf = [&]() -> Value* {
        return sem->flagRead(Flag::ZF);
    };

    auto sf = [&]() -> Value* {
        return sem->flagRead(Flag::SF);
    };

    auto of = [&]() -> Value* {
        return sem->flagRead(Flag::OF);
    };

    auto pf = [&]() -> Value* {
        return sem->flagRead(Flag::PF);
    };

    auto boolNot = [&](Value *value) -> Value* {
        return sem->m_Builder.CreateNot(value);
    };

    auto boolNotEql = [&](Value *a, Value *b) -> Value* {
        return sem->m_Builder.CreateXor(a, b);
    };

    switch (cc) {
        case CCCond::Above: {
            return boolNot(sem->m_Builder.CreateOr(cf(), zf()));
        } break;
        case CCCond::AboveEql: {
            return boolNot(cf());
        } break;
        case CCCond::Below: {
            return cf();
        } break;
        case CCCond::BelowEql: {
            return sem->m_Builder.CreateOr(cf(), zf());
        } break;
        case CCCond::Greater: {
            return boolNot(sem->m_Builder.CreateOr(zf(), boolNotEql(sf(), of())));
        } break;
        case CCCond::GreaterEql: {
            return boolNot(boolNotEql(sf(), of()));
        } break;
        case CCCond::Less: {
            return boolNotEql(sf(), of());
        } break;
        case CCCond::LessEql: {
            return sem->m_Builder.CreateOr(zf(), boolNotEql(sf(), of()));
        } break;
        case CCCond::Zero: {
            return zf();
        } break;
        case CCCond::NotZero: {
            return boolNot(zf());
        } break;
        case CCCond::Overflow: {
            return of();
        } break;
        case CCCond::NotOverflow: {
            return boolNot(of());
        } break;
        case CCCond::Parity: {
            return pf();
        } break;
        case CCCond::NotParity: {
            return boolNot(pf());
        } break;
        case CCCond::Sign: {
            return sf();
        } break;
        case CCCond::NotSign: {
            return boolNot(sf());
        } break;
    }

    std::unreachable();
}

auto Semantics_x86::HandleJz(Semantics *sem) -> std::vector<Successor> {
    return HelperConditinalJump(sem, HelperCCCond(sem, CCCond::Zero));
}

// TODO: others...

auto Semantics_x86::HandleJmp(Semantics *sem) -> std::vector<Successor> {
    auto target_addr_val = sem->opRead(0);
    if (auto target_addr_val_cte = dyn_cast<ConstantInt>(target_addr_val)) {
        auto bb = sem->getOrCreateBlock(target_addr_val_cte->getZExtValue());
        sem->m_Builder.CreateBr(bb);
    } else {
        sem->m_Builder.CreateCall(jumpHandler, { target_addr_val });
        sem->m_Builder.CreateRetVoid();
    }
    return { Successor(sem->m_Insn->address, target_addr_val) };
}

auto Semantics_x86::HandleCall(Semantics *sem) -> std::vector<Successor> {
    auto call = sem->opRead(0);
    auto ret_addr = sem->m_Insn->address + sem->m_Insn->size;
    auto ret_addr_val = sem->m_Builder.getInt64(ret_addr);
    sem->push(ret_addr_val);
    sem->m_Builder.CreateCall(callHandler, { call });
    sem->m_Builder.CreateBr(sem->getOrCreateBlock(ret_addr));
    return { Successor(sem->m_Insn->address, ret_addr_val) };
}

auto Semantics_x86::HandleRet(Semantics *sem) -> std::vector<Successor> {
    auto addr_val = sem->pop();
    if (sem->m_Insn->detail->x86.op_count != 0) {
        auto& rsp = sem->_regs.at(X86_REG_RSP);
        auto* extra = sem->m_Builder.getInt64(sem->m_Insn->detail->x86.operands[0].imm);
        sem->regWrite(rsp, sem->m_Builder.CreateAdd(sem->regRead(rsp), extra));
    }
    sem->m_Builder.CreateCall(retHandler, { addr_val });
    sem->m_Builder.CreateRetVoid();
    return { Successor(sem->m_Insn->address, addr_val) };
}

void Semantics::initializeMetadata() {
    // State field index, width in bits, and bit offset within the parent register.
    _regs = {
        {X86_REG_RAX, {"rax", 0, 64, 0}},
        {X86_REG_EAX, {"eax", 0, 32, 0}},
        {X86_REG_AX, {"ax", 0, 16, 0}},
        {X86_REG_AL, {"al", 0, 8, 0}},
        {X86_REG_AH, {"ah", 0, 8, 8}},

        {X86_REG_RBX, {"rbx", 1, 64, 0}},
        {X86_REG_EBX, {"ebx", 1, 32, 0}},
        {X86_REG_BX, {"bx", 1, 16, 0}},
        {X86_REG_BL, {"bl", 1, 8, 0}},
        {X86_REG_BH, {"bh", 1, 8, 8}},

        {X86_REG_RCX, {"rcx", 2, 64, 0}},
        {X86_REG_ECX, {"ecx", 2, 32, 0}},
        {X86_REG_CX, {"cx", 2, 16, 0}},
        {X86_REG_CL, {"cl", 2, 8, 0}},
        {X86_REG_CH, {"ch", 2, 8, 8}},

        {X86_REG_RDX, {"rdx", 3, 64, 0}},
        {X86_REG_EDX, {"edx", 3, 32, 0}},
        {X86_REG_DX, {"dx", 3, 16, 0}},
        {X86_REG_DL, {"dl", 3, 8, 0}},
        {X86_REG_DH, {"dh", 3, 8, 8}},

        {X86_REG_RSI, {"rsi", 4, 64, 0}},
        {X86_REG_ESI, {"esi", 4, 32, 0}},
        {X86_REG_SI, {"si", 4, 16, 0}},
        {X86_REG_SIL, {"sil", 4, 8, 0}},

        {X86_REG_RDI, {"rdi", 5, 64, 0}},
        {X86_REG_EDI, {"edi", 5, 32, 0}},
        {X86_REG_DI, {"di", 5, 16, 0}},
        {X86_REG_DIL, {"dil", 5, 8, 0}},

        {X86_REG_RBP, {"rbp", 6, 64, 0}},
        {X86_REG_EBP, {"ebp", 6, 32, 0}},
        {X86_REG_BP, {"bp", 6, 16, 0}},
        {X86_REG_BPL, {"bpl", 6, 8, 0}},

        {X86_REG_RSP, {"rsp", 7, 64, 0}},
        {X86_REG_ESP, {"esp", 7, 32, 0}},
        {X86_REG_SP, {"sp", 7, 16, 0}},
        {X86_REG_SPL, {"spl", 7, 8, 0}},

        {X86_REG_R8, {"r8", 8, 64, 0}},
        {X86_REG_R8D, {"r8d", 8, 32, 0}},
        {X86_REG_R8W, {"r8w", 8, 16, 0}},
        {X86_REG_R8B, {"r8b", 8, 8, 0}},

        {X86_REG_R9, {"r9", 9, 64, 0}},
        {X86_REG_R9D, {"r9d", 9, 32, 0}},
        {X86_REG_R9W, {"r9w", 9, 16, 0}},
        {X86_REG_R9B, {"r9b", 9, 8, 0}},

        {X86_REG_R10, {"r10", 10, 64, 0}},
        {X86_REG_R10D, {"r10d", 10, 32, 0}},
        {X86_REG_R10W, {"r10w", 10, 16, 0}},
        {X86_REG_R10B, {"r10b", 10, 8, 0}},

        {X86_REG_R11, {"r11", 11, 64, 0}},
        {X86_REG_R11D, {"r11d", 11, 32, 0}},
        {X86_REG_R11W, {"r11w", 11, 16, 0}},
        {X86_REG_R11B, {"r11b", 11, 8, 0}},

        {X86_REG_R12, {"r12", 12, 64, 0}},
        {X86_REG_R12D, {"r12d", 12, 32, 0}},
        {X86_REG_R12W, {"r12w", 12, 16, 0}},
        {X86_REG_R12B, {"r12b", 12, 8, 0}},

        {X86_REG_R13, {"r13", 13, 64, 0}},
        {X86_REG_R13D, {"r13d", 13, 32, 0}},
        {X86_REG_R13W, {"r13w", 13, 16, 0}},
        {X86_REG_R13B, {"r13b", 13, 8, 0}},

        {X86_REG_R14, {"r14", 14, 64, 0}},
        {X86_REG_R14D, {"r14d", 14, 32, 0}},
        {X86_REG_R14W, {"r14w", 14, 16, 0}},
        {X86_REG_R14B, {"r14b", 14, 8, 0}},

        {X86_REG_R15, {"r15", 15, 64, 0}},
        {X86_REG_R15D, {"r15d", 15, 32, 0}},
        {X86_REG_R15W, {"r15w", 15, 16, 0}},
        {X86_REG_R15B, {"r15b", 15, 8, 0}},

        {X86_REG_RIP, {"rip", 16, 64, 0}},
        {X86_REG_EIP, {"eip", 16, 32, 0}},
        {X86_REG_IP, {"ip", 16, 16, 0}},

        {X86_REG_GS, {"gs", 17, 64, 0}},
    };

    _semantics = {
        {X86_INS_MOV, &Semantics_x86::HandleMov},
        {X86_INS_MOVABS, &Semantics_x86::HandleMov},

        {X86_INS_AND, &Semantics_x86::HandleAnd},
        {X86_INS_OR, &Semantics_x86::HandleOr},
        {X86_INS_XOR, &Semantics_x86::HandleXor},

        {X86_INS_JE, &Semantics_x86::HandleJz},

        {X86_INS_JMP, &Semantics_x86::HandleJmp},
        {X86_INS_CALL, &Semantics_x86::HandleCall},
        {X86_INS_RET, &Semantics_x86::HandleRet},
    };
}

} // namespace lifter
