#include "semantics.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

namespace lifter {

namespace handlers {

using namespace llvm;

auto HandleMov(Semantics* sem) -> std::vector<Successor> {
    auto value = sem->opRead(1);
    sem->opWrite(0, value);
    return {};
}

auto HelperUpdateFlags(Semantics* sem, Value *value) -> void {
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

    sem->flagWrite(Semantics::Flag::PF, resultParityEven(value));
    sem->flagWrite(Semantics::Flag::ZF, resultIsZero(value));
    sem->flagWrite(Semantics::Flag::SF, resultSignBit(value));
}

auto HelperBinaryOP(Semantics* sem, Instruction::BinaryOps Opc) -> std::vector<Successor> {
    auto lhs = sem->opRead(0);
    auto rhs = sem->m_Builder.CreateZExtOrTrunc(sem->opRead(1), lhs->getType());
    auto res = sem->m_Builder.CreateBinOp(Opc, lhs, rhs);
    sem->opWrite(0, res);
    HelperUpdateFlags(sem, res);
    auto& ir = sem->m_Builder;
    sem->flagWrite(Semantics::CF, ir.getInt1(false));
    sem->flagWrite(Semantics::OF, ir.getInt1(false));
    sem->flagWriteUndef(Semantics::AF);
    return {};
}

auto HandleAnd(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::And);
}

auto HandleOr(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::Or);
}

auto HandleXor(Semantics *sem) -> std::vector<Successor> {
    return HelperBinaryOP(sem, Instruction::BinaryOps::Xor);
}

} // namespace handlers

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

        {X86_REG_GS, {"gs", 0, 64, 0}},
    };

    _semantics = {
        {X86_INS_MOV, handlers::HandleMov},
        {X86_INS_MOVABS, handlers::HandleMov},
        {X86_INS_AND, handlers::HandleAnd},
        {X86_INS_OR, handlers::HandleOr},
        {X86_INS_XOR, handlers::HandleXor},
    };

    // TODO: generalize this **later**
    const char* name = "__lifter_undef_af";
    auto* helper = m_Module.getFunction(name);
    auto* type = llvm::FunctionType::get(_i1Ty, {_i64Ty}, false);
    if (helper && helper->getFunctionType() != type) {
        throw std::invalid_argument("Undefined flag helper has incompatible type");
    }
    if (!helper) {
        helper = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, m_Module);
    }
    _undefFlags.emplace(AF, helper);
}

} // namespace lifter
