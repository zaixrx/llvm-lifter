#include "semantics.hpp"

#include <format>
#include <stdexcept>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/CodeGen.h>

namespace lifter {

using namespace llvm;

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
    // Undefined flag values have no observable effects when the flag is dead.
    auto* undefinedAF = _undefFlags.at(AF);
    undefinedAF->setDoesNotAccessMemory();
    undefinedAF->setDoesNotThrow();
    undefinedAF->setWillReturn();
}

// data
auto Semantics_x86::HandleMov(Semantics* sem) -> std::vector<Successor> {
    auto value = sem->opRead(1);
    sem->opWrite(0, value);
    return {};
}

// logical & arith
auto Semantics_x86::HelperLogicalOp(Semantics* sem, llvm::Instruction::BinaryOps Opc, void(Semantics_x86::*helper)(Semantics*, llvm::Value*)) -> std::vector<Successor> {
    auto lhs = sem->opRead(0);
    auto rhs = sem->m_Builder.CreateZExtOrTrunc(sem->opRead(1), lhs->getType());
    auto res = sem->m_Builder.CreateBinOp(Opc, lhs, rhs);
    sem->opWrite(0, res);
    (this->*helper)(sem, res);
    return {};
}

auto Semantics_x86::HelperArithmeticOp(Semantics* sem, llvm::Instruction::BinaryOps Opc, void(Semantics_x86::*helper)(Semantics*, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *res)) -> std::vector<Successor> {
    auto lhs = sem->opRead(0);
    auto rhs = sem->m_Builder.CreateZExtOrTrunc(sem->opRead(1), lhs->getType());
    auto res = sem->m_Builder.CreateBinOp(Opc, lhs, rhs);
    sem->opWrite(0, res);
    (this->*helper)(sem, lhs, rhs, res);
    return {};
}

auto Semantics_x86::HelperWriteCommonFlags(Semantics* sem, Value *value) -> void {
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
    sem->flagWriteUndef(Flag::AF);
}

auto Semantics_x86::HelperWriteLogicalFlags(Semantics *sem, Value *value) -> void {
    HelperWriteCommonFlags(sem, value);
    sem->flagWrite(Flag::CF, sem->m_Builder.getInt1(false));
    sem->flagWrite(Flag::OF, sem->m_Builder.getInt1(false));
}

auto Semantics_x86::HelperAddOverflow(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> llvm::Value* {
    Value *sign_mask = ConstantInt::get(lhs->getType(), 1 << (lhs->getType()->getIntegerBitWidth() - 1));
    Value *overflow_bits = sem->m_Builder.CreateAnd(sem->m_Builder.CreateXor(lhs, result), sem->m_Builder.CreateXor(rhs, result));
    return sem->m_Builder.CreateICmpEQ(
        sem->m_Builder.CreateAnd(overflow_bits, sign_mask),
        ConstantInt::get(lhs->getType(), 1)
    );
}

auto Semantics_x86::HelperSubOverflow(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> llvm::Value* {
    Value *sign_mask = ConstantInt::get(lhs->getType(), 1 << (lhs->getType()->getIntegerBitWidth() - 1));
    Value *overflow_bits = sem->m_Builder.CreateAnd(sem->m_Builder.CreateXor(lhs, rhs), sem->m_Builder.CreateXor(lhs, result));
    return sem->m_Builder.CreateICmpEQ(
        sem->m_Builder.CreateAnd(overflow_bits, sign_mask),
        ConstantInt::get(lhs->getType(), 1)
    );
}

auto Semantics_x86::HelperWriteAddFlags(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> void {
    sem->flagWrite(Flag::CF, sem->m_Builder.CreateCmp(CmpInst::Predicate::ICMP_ULE, result, rhs));
    sem->flagWrite(Flag::OF, HelperAddOverflow(sem, lhs, rhs, result));
    HelperWriteCommonFlags(sem, result); // TODO: af has special handling here
}

auto Semantics_x86::HelperWriteSubFlags(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> void {
    sem->flagWrite(Flag::CF, sem->m_Builder.CreateCmp(CmpInst::Predicate::ICMP_ULE, lhs, rhs));
    sem->flagWrite(Flag::OF, HelperSubOverflow(sem, lhs, rhs, result));
    HelperWriteCommonFlags(sem, result); // TODO: af has special handling here
}

auto Semantics_x86::HandleAnd(Semantics *sem) -> std::vector<Successor> {
    return HelperLogicalOp(sem, Instruction::BinaryOps::And, &Semantics_x86::HelperWriteCommonFlags);
}

auto Semantics_x86::HandleOr(Semantics *sem) -> std::vector<Successor> {
    return HelperLogicalOp(sem, Instruction::BinaryOps::Or, &Semantics_x86::HelperWriteCommonFlags);
}

auto Semantics_x86::HandleXor(Semantics *sem) -> std::vector<Successor> {
    return HelperLogicalOp(sem, Instruction::BinaryOps::Xor, &Semantics_x86::HelperWriteCommonFlags);
}

auto Semantics_x86::HandleCmp(Semantics *sem) -> std::vector<Successor> {
    auto lhs = sem->opRead(0);
    auto rhs = sem->opRead(1);
    rhs = sem->m_Builder.CreateZExtOrTrunc(rhs, lhs->getType());
    auto res = sem->m_Builder.CreateSub(lhs, rhs);
    HelperWriteSubFlags(sem, lhs, rhs, res);
    return {};
}

auto Semantics_x86::HandleAdd(Semantics *sem) -> std::vector<Successor> {
    HelperArithmeticOp(sem, llvm::Instruction::BinaryOps::Add, &Semantics_x86::HelperWriteAddFlags);
    return {};
}

auto Semantics_x86::HandleSub(Semantics *sem) -> std::vector<Successor> {
    HelperArithmeticOp(sem, llvm::Instruction::BinaryOps::Add, &Semantics_x86::HelperWriteSubFlags);
    return {};
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

auto Semantics_x86::GetCCCond(Semantics *sem, CCCond cc) {
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
    return HelperConditinalJump(sem, GetCCCond(sem, CCCond::Zero));
}

auto Semantics_x86::HandleJnz(Semantics *sem) -> std::vector<Successor> {
    return HelperConditinalJump(sem, GetCCCond(sem, CCCond::NotZero));
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
        auto rsp = sem->_regs.at(X86_REG_RSP);
        auto* extra = sem->m_Builder.getInt64(sem->m_Insn->detail->x86.operands[0].imm);
        sem->regWrite(rsp, sem->m_Builder.CreateAdd(sem->regRead(rsp), extra));
    }
    sem->m_Builder.CreateCall(retHandler, { addr_val });
    sem->m_Builder.CreateRetVoid();
    return { Successor(sem->m_Insn->address, addr_val) };
}

void Semantics::initializeMetadata() {
    _regs = {
        {X86_REG_RAX, {"rax", (offsetof(X86State, rax) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EAX, {"eax", (offsetof(X86State, rax) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_AX, {"ax", (offsetof(X86State, rax) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_AL, {"al", (offsetof(X86State, rax) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_AH, {"ah", (offsetof(X86State, rax) / sizeof(std::uint64_t)), 8, 8}},
        {X86_REG_RBX, {"rbx", (offsetof(X86State, rbx) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EBX, {"ebx", (offsetof(X86State, rbx) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_BX, {"bx", (offsetof(X86State, rbx) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_BL, {"bl", (offsetof(X86State, rbx) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_BH, {"bh", (offsetof(X86State, rbx) / sizeof(std::uint64_t)), 8, 8}},
        {X86_REG_RCX, {"rcx", (offsetof(X86State, rcx) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_ECX, {"ecx", (offsetof(X86State, rcx) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_CX, {"cx", (offsetof(X86State, rcx) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_CL, {"cl", (offsetof(X86State, rcx) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_CH, {"ch", (offsetof(X86State, rcx) / sizeof(std::uint64_t)), 8, 8}},
        {X86_REG_RDX, {"rdx", (offsetof(X86State, rdx) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EDX, {"edx", (offsetof(X86State, rdx) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_DX, {"dx", (offsetof(X86State, rdx) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_DL, {"dl", (offsetof(X86State, rdx) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_DH, {"dh", (offsetof(X86State, rdx) / sizeof(std::uint64_t)), 8, 8}},
        {X86_REG_RSI, {"rsi", (offsetof(X86State, rsi) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_ESI, {"esi", (offsetof(X86State, rsi) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_SI, {"si", (offsetof(X86State, rsi) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_SIL, {"sil", (offsetof(X86State, rsi) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_RDI, {"rdi", (offsetof(X86State, rdi) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EDI, {"edi", (offsetof(X86State, rdi) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_DI, {"di", (offsetof(X86State, rdi) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_DIL, {"dil", (offsetof(X86State, rdi) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_RBP, {"rbp", (offsetof(X86State, rbp) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EBP, {"ebp", (offsetof(X86State, rbp) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_BP, {"bp", (offsetof(X86State, rbp) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_BPL, {"bpl", (offsetof(X86State, rbp) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_RSP, {"rsp", (offsetof(X86State, rsp) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_ESP, {"esp", (offsetof(X86State, rsp) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_SP, {"sp", (offsetof(X86State, rsp) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_SPL, {"spl", (offsetof(X86State, rsp) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R8, {"r8", (offsetof(X86State, r8) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R8D, {"r8d", (offsetof(X86State, r8) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R8W, {"r8w", (offsetof(X86State, r8) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R8B, {"r8b", (offsetof(X86State, r8) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R9, {"r9", (offsetof(X86State, r9) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R9D, {"r9d", (offsetof(X86State, r9) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R9W, {"r9w", (offsetof(X86State, r9) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R9B, {"r9b", (offsetof(X86State, r9) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R10, {"r10", (offsetof(X86State, r10) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R10D, {"r10d", (offsetof(X86State, r10) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R10W, {"r10w", (offsetof(X86State, r10) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R10B, {"r10b", (offsetof(X86State, r10) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R11, {"r11", (offsetof(X86State, r11) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R11D, {"r11d", (offsetof(X86State, r11) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R11W, {"r11w", (offsetof(X86State, r11) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R11B, {"r11b", (offsetof(X86State, r11) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R12, {"r12", (offsetof(X86State, r12) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R12D, {"r12d", (offsetof(X86State, r12) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R12W, {"r12w", (offsetof(X86State, r12) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R12B, {"r12b", (offsetof(X86State, r12) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R13, {"r13", (offsetof(X86State, r13) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R13D, {"r13d", (offsetof(X86State, r13) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R13W, {"r13w", (offsetof(X86State, r13) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R13B, {"r13b", (offsetof(X86State, r13) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R14, {"r14", (offsetof(X86State, r14) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R14D, {"r14d", (offsetof(X86State, r14) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R14W, {"r14w", (offsetof(X86State, r14) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R14B, {"r14b", (offsetof(X86State, r14) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_R15, {"r15", (offsetof(X86State, r15) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_R15D, {"r15d", (offsetof(X86State, r15) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_R15W, {"r15w", (offsetof(X86State, r15) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_R15B, {"r15b", (offsetof(X86State, r15) / sizeof(std::uint64_t)), 8, 0}},
        {X86_REG_RIP, {"rip", (offsetof(X86State, rip) / sizeof(std::uint64_t)), 64, 0}},
        {X86_REG_EIP, {"eip", (offsetof(X86State, rip) / sizeof(std::uint64_t)), 32, 0}},
        {X86_REG_IP, {"ip", (offsetof(X86State, rip) / sizeof(std::uint64_t)), 16, 0}},
        {X86_REG_GS, {"gs", (offsetof(X86State, gs) / sizeof(std::uint64_t)), 64, 0}}
    };

    _semantics = {
        {X86_INS_MOV, &Semantics_x86::HandleMov},
        {X86_INS_MOVABS, &Semantics_x86::HandleMov},

        {X86_INS_AND, &Semantics_x86::HandleAnd},
        {X86_INS_OR, &Semantics_x86::HandleOr},
        {X86_INS_XOR, &Semantics_x86::HandleXor},

        {X86_INS_ADD, &Semantics_x86::HandleAdd},
        {X86_INS_SUB, &Semantics_x86::HandleSub},

        {X86_INS_JE, &Semantics_x86::HandleJz},
        {X86_INS_JNE, &Semantics_x86::HandleJnz},
        {X86_INS_CMP, &Semantics_x86::HandleCmp},

        {X86_INS_JMP, &Semantics_x86::HandleJmp},
        {X86_INS_CALL, &Semantics_x86::HandleCall},
        {X86_INS_RET, &Semantics_x86::HandleRet},
    };
}

} // namespace lifter
