#include "semantics.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

#include <print>
#include <format>

namespace lifter {

using namespace llvm;

namespace handlers {

auto HandleMov(Semantics& sem) -> std::vector<Successor> {
    auto value = sem.opRead(1);
    sem.opWrite(0, value);
    return {};
}

auto HelperBinaryOP(Semantics& sem) -> std::vector<Successor> {
    auto lhs = sem.opRead(1);
    auto rhs = sem.opRead(0);
    sem.
    return {};
}

} // namespace handlers

Semantics::Semantics(Module& mod) : m_Module(mod), m_Context(mod.getContext()), m_Builder(m_Context) {
    createTypes();
    initializeMetadata();

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &m_Capstone) != CS_ERR_OK) {
        throw std::runtime_error("Failed to initialize Capstone");
    }

    if (cs_option(m_Capstone, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
        cs_close(&m_Capstone);
        throw std::runtime_error("Failed to configure Capstone");
    }
}

Semantics::~Semantics() {
    cs_close(&m_Capstone);
}

auto Semantics::begin(std::uintptr_t addr) -> Function* {
    const auto name = std::format("lifted_0x{:x}", addr);
    if (m_Module.getFunction(name)) {
        throw std::invalid_argument(std::format("lifted function already exists: {}", name));
    }

    m_Function = Function::Create(m_LiftedTy, Function::ExternalLinkage, name, m_Module);
    m_Function->getArg(0)->setName("state");
    m_Function->getArg(1)->setName("memory");
    m_Function->addParamAttr(0, Attribute::AttrKind::NoAlias);
    m_Function->addParamAttr(1, Attribute::AttrKind::NoAlias);

    m_Blocks.clear();
    m_RegPtrs.clear();
    m_FlagPtrs.clear();

    BasicBlock* entry = BasicBlock::Create(m_Context, "initialize", m_Function);
    BasicBlock* destination = getOrCreateBlock(addr);
    m_Builder.SetInsertPoint(entry);
    m_Builder.CreateBr(destination);

    return m_Function;
}

auto Semantics::getOrCreateBlock(std::uintptr_t addr) -> BasicBlock* {
    if (m_Blocks.contains(addr)) {
        return m_Blocks[addr];
    }
    BasicBlock* block = BasicBlock::Create(m_Context, std::format("insn_0x{:x}", addr), m_Function);
    IRBuilder<> temp(block);
    temp.CreateRetVoid();
    m_Blocks[addr] = block;
    return block;
}

auto Semantics::getOrCreateRegPointer(Register &reg) -> Value* {
    if (!m_RegPtrs.contains(reg.index)) {
        return m_RegPtrs[reg.index];
    }
    IRBuilder<> temp(m_Function->getEntryBlock().getTerminator());
    Value* pointer = temp.CreateStructGEP(m_StateTy, m_Function->getArg(0), reg.index, reg.name);
    m_RegPtrs.emplace(reg.index, pointer);
    return pointer;
}

auto Semantics::regRead(Register &reg) -> Value* {
    Value* regPtr = getOrCreateRegPointer(reg);
    Value* value = m_Builder.CreateAlignedLoad(_i64Ty, regPtr, Align(8));
    if (reg.offset) {
        value = m_Builder.CreateLShr(value, m_Builder.getInt64(reg.offset));
    }
    if (reg.size != 64) {
        value = m_Builder.CreateTrunc(value, m_Builder.getIntNTy(reg.size));
    }
    return value;
}

auto Semantics::regWrite(Register &reg, Value* value) -> void {
    Value* pointer = getOrCreateRegPointer(reg);

    // Make same active number of bits as `reg`
    value = m_Builder.CreateZExtOrTrunc(value, m_Builder.getIntNTy(reg.size));

    if (reg.size != 64) {
        // Zero extend to 64 bit register
        value = m_Builder.CreateZExt(value, _i64Ty);

        if (reg.size != 32) {
            Value* old = m_Builder.CreateAlignedLoad(_i64Ty, pointer, Align(8));
            const uint64_t inv_mask = ~(((uint64_t{1} << reg.size) - 1) << reg.offset);
            old = m_Builder.CreateAnd(old, m_Builder.getInt64(inv_mask));
            value = m_Builder.CreateShl(value, m_Builder.getInt64(reg.offset));
            value = m_Builder.CreateOr(old, value);
        }
    }

    m_Builder.CreateAlignedStore(value, pointer, Align(8));
}

auto Semantics::opRead(unsigned index) -> llvm::Value* {
    auto op = m_Insn->detail->x86.operands[index];

    switch (op.type) {
    case X86_OP_REG: {
        return regRead(_regs.at(op.reg));
    } break;

	case X86_OP_IMM: {
        auto ty = IntegerType::get(m_Context, op.size * 8);
        return llvm::ConstantInt::get(ty, op.imm);
    } break;

	case X86_OP_MEM: {
        uintptr_t addr = op.mem.base + (op.mem.index * op.mem.scale) + op.mem.disp; // TODO: wtf
        return memRead(addr, IntegerType::get(m_Context, op.size * 8));
    } break;

    default:
        std::unreachable();
    }
}

auto Semantics::opWrite(unsigned index, Value *value) -> void {
    auto op = m_Insn->detail->x86.operands[index];

    switch (op.type) {
    case X86_OP_REG: {
        regWrite(_regs.at(op.reg), value);
    } break;

	case X86_OP_IMM: {
        throw std::invalid_argument(std::format("Cannot write to CS_OP_IMM"));
    } break;

	case X86_OP_MEM: {
        uintptr_t addr = op.mem.base + (op.mem.index * op.mem.scale) + op.mem.disp; // TODO: wtf
        memWrite(addr, value);
    } break;

    default:
        std::unreachable();
    }
}

auto Semantics::memRead(uintptr_t addr, llvm::Type *ty) -> llvm::Value* {
    Value *mem = m_Function->getArg(1);
    Value *ptr = m_Builder.CreateGEP(_i8Ty, mem, { llvm::ConstantInt::get(_i64Ty, addr) });
    return m_Builder.CreateLoad(ty, ptr);
}

auto Semantics::memWrite(uintptr_t addr, llvm::Value* value) -> void {
    Value *mem = m_Function->getArg(1);
    Value *ptr = m_Builder.CreateGEP(_i8Ty, mem, { llvm::ConstantInt::get(_i64Ty, addr) });
    m_Builder.CreateStore(value, ptr);
}

auto Semantics::getOrCreateFlagPointer(Flag flag) -> llvm::Value* {
    if (m_FlagPtrs.contains(flag)) {
        return m_FlagPtrs[flag];
    }
    IRBuilder<> temp(m_Function->getEntryBlock().getTerminator());
    Value* pointer = temp.CreateStructGEP(m_StateTy, m_Function->getArg(0), 17 + flag, std::format("eflags_{}", unsigned{flag}));
    m_RegPtrs.emplace(flag, pointer);
    return pointer;
}


auto Semantics::flagRead(Flag flag) -> llvm::Value* {
    Value *ptr = getOrCreateFlagPointer(flag);
    Value *val = m_Builder.CreateLoad(m_StateTy, ptr);
    return m_Builder.CreateICmpEQ(val, llvm::ConstantInt::get(_i8Ty, 0));
}

auto Semantics::flagWrite(Flag flag, llvm::Value* value) -> void {
    Type *ty = value->getType();
    if (ty != _i8Ty) {
        assert(ty == _i1Ty && "fuck we doing bro🥀🥀🥀...");
        value = m_Builder.CreateZExt(value, _i8Ty);
    }
    Value *ptr = getOrCreateFlagPointer(flag);
    m_Builder.CreateStore(value, ptr);
}

auto Semantics::liftBytes(uintptr_t addr, std::span<std::uint8_t> code) -> std::vector<Successor> {
    if (!m_Function) {
        begin(addr);
    }

    auto insn = [&] () {
        cs_insn* insn = nullptr;
        if (cs_disasm(m_Capstone, code.data(), code.size(), addr, 1, &insn) != 1) {
            throw std::invalid_argument("Failed to decode instruction at: " + std::format("0x{:x}", addr));
        }
        if (true) {
            std::println("0x{:X}:\t{}\t\t{}\n", insn[0].address, insn[0].mnemonic, insn[0].op_str);
        }
        std::unique_ptr<cs_insn, InsnFini> owned(insn);
        return owned;
    }();

    BasicBlock* block = getOrCreateBlock(addr);
    if (block->front().getOpcode() != Instruction::Ret) {
        return {};
    }
    Instruction* placeholder = block->getTerminator();
    m_Builder.SetInsertPoint(placeholder);
    
    auto id = static_cast<x86_insn>(insn->id);
    if (!_semantics.contains(id)) {
        throw std::invalid_argument(std::format("Unsupported instruction, got: {}", insn->mnemonic));
    }

    /* TODO
    for (unsigned i = 0; i < insn->detail->x86.op_count; ++i) {
        const auto& op = insn->detail->x86.operands[i];
        if (op.type == X86_OP_REG && !_regs.contains(op.reg)) {
            throw std::invalid_argument(std::format("Unsupported register, got: {}", static_cast<unsigned>(op.reg)));
        }
    }
    */

    m_Insn = std::move(insn);
    std::vector<Successor> succs = _semantics[id](this);

    if (!succs.size()) {
        uint64_t fallthrough = addr + m_Insn->size;
        Value *fallthrough_Value = m_Builder.getInt64(fallthrough);

        BasicBlock* next = getOrCreateBlock(fallthrough);
        m_Builder.CreateBr(next);

        succs.push_back({ addr, fallthrough_Value });
    }

    placeholder->eraseFromParent(); // remove the original ret, not the new branch

    if (verifyModule(m_Module, &errs())) {
        throw std::runtime_error("Failed to verify module");
    }

    return succs;
}

void Semantics::createTypes() {
    _i1Ty = m_Builder.getInt1Ty();
    _i8Ty = m_Builder.getInt8Ty();
    _i16Ty = m_Builder.getInt16Ty();
    _i32Ty = m_Builder.getInt32Ty();
    _i64Ty = m_Builder.getInt64Ty();

    auto namedStruct = [] (LLVMContext& context, StringRef name, ArrayRef<Type*> fields) -> StructType* {
        if (auto* existing = StructType::getTypeByName(context, name)) {
            if (existing->isOpaque() || existing->elements() != fields) {
                throw std::runtime_error("conflicting LLVM state type: " + name.str());
            }
            return existing;
        }
        return StructType::create(context, fields, name);
    };

    std::vector<Type*> flag_fields(17, _i8Ty);
    auto* flags = namedStruct(m_Context, "Flags", flag_fields);
    auto* segment = namedStruct(m_Context, "Segment", {_i64Ty, _i32Ty, _i16Ty, _i16Ty});
    auto* x87 = namedStruct(
        m_Context, "X87", { ArrayType::get(ArrayType::get(_i8Ty, 10), 8), _i16Ty, _i16Ty, _i16Ty, _i16Ty, _i64Ty, _i64Ty}
    );

    std::vector<Type*> fields(17, _i64Ty); // 17 GPRs
    fields.insert(
        fields.end(),
        {
            flags, ArrayType::get(segment, 6), x87,
            ArrayType::get(ArrayType::get(_i8Ty, 64), 32),
            ArrayType::get(_i64Ty, 8), _i32Ty, _i32Ty
        }
    );
    m_StateTy = namedStruct(m_Context, "State", fields);

    Type* pointerTy = PointerType::getUnqual(m_Context);
    m_LiftedTy = FunctionType::get(m_Builder.getVoidTy(), { pointerTy, pointerTy }, false);
}

} // namespace lifter
