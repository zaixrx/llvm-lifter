#pragma once

#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <cstddef>
#include <cstdint>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <map>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace lifter {

class Semantics;

struct Successor {
    std::uint64_t src;
    llvm::Value* dst;
};

struct InsnFini {
    void operator()(cs_insn* instruction) const noexcept { cs_free(instruction, 1); }
};

struct X86State {
    std::uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp;
    std::uint64_t r8, r9, r10, r11, r12, r13, r14, r15, rip, gs;
    std::uint8_t cf, pf, af, zf, sf, of;
};

class Semantics_x86 {
public:
    enum Register : std::size_t {
        RAX = offsetof(X86State, rax),
        RBX = offsetof(X86State, rbx),
        RCX = offsetof(X86State, rcx),
        RDX = offsetof(X86State, rdx),
        RSI = offsetof(X86State, rsi),
        RDI = offsetof(X86State, rdi),
        RBP = offsetof(X86State, rbp),
        RSP = offsetof(X86State, rsp),
        R8 = offsetof(X86State, r8),
        R9 = offsetof(X86State, r9),
        R10 = offsetof(X86State, r10),
        R11 = offsetof(X86State, r11),
        R12 = offsetof(X86State, r12),
        R13 = offsetof(X86State, r13),
        R14 = offsetof(X86State, r14),
        R15 = offsetof(X86State, r15),
        RIP = offsetof(X86State, rip),
        GS = offsetof(X86State, gs)
    };

    enum Flag {
        CF, PF, AF, ZF, SF, OF
    };

    explicit Semantics_x86(Semantics*);
    Semantics_x86(const Semantics_x86&) = delete;
    Semantics_x86& operator=(const Semantics_x86&) = delete;

    auto HandleMov(Semantics* sem) -> std::vector<Successor>;

    auto HandleAnd(Semantics *sem) -> std::vector<Successor>;
    auto HandleOr(Semantics *sem) -> std::vector<Successor>;
    auto HandleXor(Semantics *sem) -> std::vector<Successor>;

    auto HandleAdd(Semantics *sem) -> std::vector<Successor>;
    auto HandleSub(Semantics *sem) -> std::vector<Successor>;

    auto HandleCmp(Semantics *sem) -> std::vector<Successor>;
    auto HandleJz(Semantics *sem) -> std::vector<Successor>;
    auto HandleJnz(Semantics *sem) -> std::vector<Successor>;

    auto HandleJmp(Semantics *sem) -> std::vector<Successor>;
    auto HandleCall(Semantics *sem) -> std::vector<Successor>;
    auto HandleRet(Semantics *sem) -> std::vector<Successor>;

private:
    enum CCCond {
        Above, // u
        AboveEql,
        Below,
        BelowEql,
        Greater, // s
        GreaterEql,
        Less,
        LessEql,
        Zero,
        NotZero,
        Overflow,
        NotOverflow,
        Parity,
        NotParity,
        Sign,
        NotSign
    };

    auto HelperLogicalOp(Semantics* sem, llvm::Instruction::BinaryOps Opc, void(Semantics_x86::*helper)(Semantics*, llvm::Value*)) -> std::vector<Successor>;
    auto HelperArithmeticOp(Semantics* sem, llvm::Instruction::BinaryOps Opc, void(Semantics_x86::*helper)(Semantics*, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *res)) -> std::vector<Successor>;

    auto HelperConditinalJump(Semantics *sem, llvm::Value *cond) -> std::vector<Successor>;
    auto GetCCCond(Semantics *sem, CCCond cc);

    auto HelperWriteCommonFlags(Semantics* sem, llvm::Value *value) -> void; 
    auto HelperWriteLogicalFlags(Semantics* sem, llvm::Value *value) -> void; 
    auto HelperAddOverflow(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> llvm::Value*;
    auto HelperSubOverflow(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> llvm::Value*;
    auto HelperWriteAddFlags(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> void;
    auto HelperWriteSubFlags(Semantics *sem, llvm::Value *lhs, llvm::Value *rhs, llvm::Value *result) -> void;

    llvm::Function *jumpHandler;
    llvm::Function *callHandler;
    llvm::Function *retHandler;

    std::map<Flag, llvm::Function*> _undefFlags;
    friend class Semantics;
};

class Semantics {
public:
    explicit Semantics(llvm::Module& mod);
    ~Semantics();
    Semantics(const Semantics&) = delete;
    Semantics& operator=(const Semantics&) = delete;

    auto begin(uintptr_t addr) -> llvm::Function*;
    std::vector<Successor> liftBytes(uintptr_t addr, std::span<std::uint8_t> code, bool optimizeLiftedFunction = false);

private:
    struct Register {
        std::string_view name;
        size_t index, size, bitOffset;
    };

    void createTypes();
    void initializeMetadata();

    auto getOrCreateBlock(uintptr_t address) -> llvm::BasicBlock*;

    auto opRead(unsigned index) -> llvm::Value*;
    auto opWrite(unsigned index, llvm::Value*) -> void;
    auto getOrCreateRegPointer(const Register &reg) -> llvm::Value*;
    auto regRead(const Register &reg) -> llvm::Value*;
    auto regWrite(const Register &reg, llvm::Value* value) -> void;
    auto memAddress(const x86_op_mem& op) -> llvm::Value*;
    auto memRead(llvm::Value *addr, llvm::Type*) -> llvm::Value*;
    auto memWrite(llvm::Value *addr, llvm::Value* value) -> void;
    auto getOrCreateFlagPointer(Semantics_x86::Flag flag) -> llvm::Value*;
    auto flagRead(Semantics_x86::Flag flag) -> llvm::Value*;
    auto flagWrite(Semantics_x86::Flag flag, llvm::Value *value) -> void;
    auto flagWriteUndef(Semantics_x86::Flag flag) -> void;
    auto push(llvm::Value *value) -> void;
    auto pop() -> llvm::Value*;

    // Global State
    llvm::Module& m_Module;
    llvm::LLVMContext& m_Context;
    llvm::IRBuilder<> m_Builder;
    csh m_Capstone{};

    // Local State
    llvm::Function* m_Function{};
    std::unique_ptr<cs_insn, InsnFini> m_Insn;
    std::map<unsigned, llvm::Value*> m_FlagPtrs;
    std::map<unsigned, llvm::Value*> m_RegPtrs;
    std::map<std::uint64_t, llvm::BasicBlock*> m_Blocks;

    // Arch info
    Semantics_x86 _x86;
    std::map<x86_reg, Register> _regs;
    std::map<x86_insn, std::vector<Successor>(Semantics_x86::*)(Semantics*)> _semantics;
    friend class Semantics_x86;

    // Types
    llvm::StructType* m_StateTy{};
    llvm::FunctionType* m_LiftedTy{};
    llvm::Type *_i64Ty, *_i32Ty, *_i16Ty, *_i8Ty, *_i1Ty;
};
} // namespace lifter
