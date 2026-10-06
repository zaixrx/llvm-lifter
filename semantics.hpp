#pragma once

#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <map>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace lifter {

struct Successor {
    std::uint64_t src;
    llvm::Value* dst;
};

class Semantics;

typedef auto (*SemanticHandler_T) (Semantics*) -> std::vector<Successor>;

struct InsnFini {
    void operator()(cs_insn* instruction) const noexcept { cs_free(instruction, 1); }
};

class Semantics {
public:
    explicit Semantics(llvm::Module& mod);
    ~Semantics();
    Semantics(const Semantics&) = delete;
    Semantics& operator=(const Semantics&) = delete;

    auto begin(uintptr_t addr) -> llvm::Function*;
    std::vector<Successor> liftBytes(uintptr_t addr, std::span<std::uint8_t> code);

// private:
    struct Register {
        std::string_view name;
        unsigned index, size, offset;
    };

    enum Flag {
        CF, PF, AF, ZF, SF, OF
    };

    void createTypes();
    void initializeMetadata();

    auto getOrCreateBlock(uintptr_t address) -> llvm::BasicBlock*;

    auto opRead(unsigned index) -> llvm::Value*;
    auto opWrite(unsigned index, llvm::Value*) -> void;
    auto getOrCreateRegPointer(Register &reg) -> llvm::Value*;
    auto regRead(Register &reg) -> llvm::Value*;
    auto regWrite(Register &reg, llvm::Value* value) -> void;
    auto memAddress(const x86_op_mem& op) -> llvm::Value*;
    auto memRead(llvm::Value *addr, llvm::Type*) -> llvm::Value*;
    auto memWrite(llvm::Value *addr, llvm::Value* value) -> void;
    auto getOrCreateFlagPointer(Flag flag) -> llvm::Value*;
    auto flagRead(Flag flag) -> llvm::Value*;
    auto flagWrite(Flag flag, llvm::Value *value) -> void;
    auto flagWriteUndef(Flag flag) -> void;

    // Global State
    llvm::Module& m_Module;
    llvm::LLVMContext& m_Context;
    llvm::IRBuilder<> m_Builder;
    csh m_Capstone{};

    // Metadata
    std::map<x86_reg, Register> _regs;
    std::map<x86_insn, SemanticHandler_T> _semantics;
    std::map<Flag, llvm::Function*> _undefFlags;

    // Local State
    llvm::Function* m_Function{};
    std::map<unsigned, llvm::Value*> m_FlagPtrs;
    std::map<unsigned, llvm::Value*> m_RegPtrs;
    std::map<std::uint64_t, llvm::BasicBlock*> m_Blocks;
    std::unique_ptr<cs_insn, InsnFini> m_Insn;

    // Types
    llvm::StructType* m_StateTy{};
    llvm::FunctionType* m_LiftedTy{};
    llvm::Type *_i64Ty, *_i32Ty, *_i16Ty, *_i8Ty, *_i1Ty;
};

} // namespace lifter
