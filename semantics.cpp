#include "semantics.hpp"
#include "optimizer.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

#include <print>
#include <format>
#include <stdexcept>

namespace lifter {

using namespace llvm;

Semantics::Semantics(Module& mod) : m_Module(mod), m_Context(mod.getContext()), m_Builder(m_Context), _x86(this) {
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

auto Semantics::getOrCreateRegPointer(const Register &reg) -> Value* {
    if (m_RegPtrs.contains(reg.index)) {
        return m_RegPtrs[reg.index];
    }
    IRBuilder<> temp(m_Function->getEntryBlock().getTerminator());
    Value* pointer = temp.CreateStructGEP(m_StateTy, m_Function->getArg(0), reg.index, reg.name);
    m_RegPtrs[reg.index] = pointer;
    return pointer;
}

auto Semantics::regRead(const Register &reg) -> Value* {
    Value* regPtr = getOrCreateRegPointer(reg);
    Value* value = m_Builder.CreateAlignedLoad(_i64Ty, regPtr, Align(8));
    if (reg.bitOffset) {
        value = m_Builder.CreateLShr(value, m_Builder.getInt64(reg.bitOffset));
    }
    if (reg.size != 64) {
        value = m_Builder.CreateTrunc(value, m_Builder.getIntNTy(reg.size));
    }
    return value;
}

auto Semantics::regWrite(const Register &reg, Value* value) -> void {
    Value* pointer = getOrCreateRegPointer(reg);
    // Make same active number of bits as `reg`
    value = m_Builder.CreateZExtOrTrunc(value, m_Builder.getIntNTy(reg.size));
    if (reg.size != 64) {
        // Zero extend to 64 bit register
        value = m_Builder.CreateZExt(value, _i64Ty);
        if (reg.size != 32) {
            Value* old = m_Builder.CreateAlignedLoad(_i64Ty, pointer, Align(8));
            const uint64_t inv_mask = ~(((uint64_t{1} << reg.size) - 1) << reg.bitOffset);
            old = m_Builder.CreateAnd(old, m_Builder.getInt64(inv_mask));
            value = m_Builder.CreateShl(value, m_Builder.getInt64(reg.bitOffset));
            value = m_Builder.CreateOr(old, value);
        }
    }
    m_Builder.CreateAlignedStore(value, pointer, Align(8));
}

auto Semantics::opRead(unsigned index) -> Value* {
    auto op = m_Insn->detail->x86.operands[index];

    switch (op.type) {
    case X86_OP_REG: {
        return regRead(_regs.at(op.reg));
    } break;

	case X86_OP_IMM: {
        auto ty = IntegerType::get(m_Context, op.size * 8);
        return ConstantInt::get(ty, op.imm);
    } break;

	case X86_OP_MEM: {
        return memRead(memAddress(op.mem), IntegerType::get(m_Context, op.size * 8));
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
        value = m_Builder.CreateZExtOrTrunc(value, m_Builder.getIntNTy(op.size * 8));
        memWrite(memAddress(op.mem), value);
    } break;

    default:
        std::unreachable();
    }
}

auto Semantics::memAddress(const x86_op_mem& mem) -> Value* {
    Value* addr = nullptr;
    Type* addrTy = IntegerType::getIntNTy(m_Context, m_Insn->detail->x86.addr_size * 8);

    if (mem.index != X86_REG_INVALID) {
        auto* index = m_Builder.CreateZExtOrTrunc(regRead(_regs.at(mem.index)), addrTy);
        addr = m_Builder.CreateMul(index, ConstantInt::get(addrTy, mem.scale));
    }
    
    if (mem.base != X86_REG_INVALID) {
        Value *baseValue = nullptr;
        if (mem.base == X86_REG_RIP || mem.base == X86_REG_EIP) {
            baseValue = ConstantInt::get(addrTy, m_Insn->address + m_Insn->size);
        } else {
            baseValue = m_Builder.CreateZExtOrTrunc(regRead(_regs.at(mem.base)), addrTy);
        }
        addr = addr ? m_Builder.CreateAdd(addr, baseValue) : baseValue;
    }

    if (mem.disp != 0) {
        Value *dispValue = ConstantInt::get(addrTy, mem.disp);
        addr = addr ? m_Builder.CreateAdd(addr, dispValue) : dispValue;
    }

    // to not dereference nullptr, e.g [0]
    if (!addr) {
        addr = ConstantInt::get(addrTy, 0);
    }

    if (addr->getType() != _i64Ty) {
        addr = m_Builder.CreateZExt(addr, _i64Ty);
    }

    if (mem.segment == X86_REG_GS) {
        Value *base = regRead(_regs.at(mem.segment));
        addr = m_Builder.CreateAdd(addr, base);
    }

    return addr;
}

auto Semantics::memRead(Value *addr, Type *ty) -> Value* {
    Value *mem = m_Function->getArg(1);
    Value *ptr = m_Builder.CreateGEP(_i8Ty, mem, {addr});
    return m_Builder.CreateAlignedLoad(ty, ptr, Align(1));
}

auto Semantics::memWrite(Value *addr, Value* value) -> void {
    Value *mem = m_Function->getArg(1);
    Value *ptr = m_Builder.CreateGEP(_i8Ty, mem, {addr});
    m_Builder.CreateAlignedStore(value, ptr, Align(1));
}

auto Semantics::getOrCreateFlagPointer(Semantics_x86::Flag flag) -> Value* {
    if (m_FlagPtrs.contains(flag)) {
        return m_FlagPtrs[flag];
    }
    IRBuilder<> temp(m_Function->getEntryBlock().getTerminator());
    const auto fieldIndex = offsetof(X86State, cf) / sizeof(std::uint64_t) + unsigned{flag};
    Value* ptr = temp.CreateStructGEP(m_StateTy, m_Function->getArg(0), fieldIndex, std::format("eflags_{}", unsigned{flag}));
    m_FlagPtrs.emplace(flag, ptr);
    return ptr;
}

auto Semantics::flagRead(Semantics_x86::Flag flag) -> Value* {
    Value *ptr = getOrCreateFlagPointer(flag);
    Value *val = m_Builder.CreateAlignedLoad(_i8Ty, ptr, Align(1));
    return m_Builder.CreateICmpNE(val, ConstantInt::get(_i8Ty, 0));
}

auto Semantics::flagWrite(Semantics_x86::Flag flag, Value* value) -> void {
    Type *ty = value->getType();
    if (ty != _i8Ty) {
        assert(ty == _i1Ty && "fuck we doing bro🥀🥀🥀...");
        value = m_Builder.CreateZExt(value, _i8Ty);
    }
    Value *ptr = getOrCreateFlagPointer(flag);
    m_Builder.CreateStore(value, ptr);
}

auto Semantics::flagWriteUndef(Semantics_x86::Flag flag) -> void {
    if (!_x86._undefFlags.contains(flag)) {
        throw std::invalid_argument(std::format("undefined flag handler is not implemented for flag {}", unsigned{flag}));
    }
    Function* helper = _x86._undefFlags.at(flag);
    auto value = m_Builder.CreateCall(helper, { ConstantInt::get(_i64Ty, m_Insn->address) });
    flagWrite(flag, value);
}

auto Semantics::push(Value* value) -> void {
    auto rsp = _regs.at(X86_REG_RSP);
    auto* addr = m_Builder.CreateSub(regRead(rsp), m_Builder.getInt64(8));
    regWrite(rsp, addr);
    memWrite(addr, m_Builder.CreateZExtOrTrunc(value, _i64Ty));
}

auto Semantics::pop() -> Value* {
    auto rsp = _regs.at(X86_REG_RSP);
    auto* addr = regRead(rsp);
    auto* value = memRead(addr, _i64Ty);
    regWrite(rsp, m_Builder.CreateAdd(addr, m_Builder.getInt64(8)));
    return value;
}

auto Semantics::liftBytes(uintptr_t addr, std::span<std::uint8_t> code, bool optimizeLiftedFunction) -> std::vector<Successor> {
    if (!m_Function) {
        begin(addr);
    }

    auto insn = [&] () {
        cs_insn* insn = nullptr;
        if (cs_disasm(m_Capstone, code.data(), code.size(), addr, 1, &insn) != 1) {
            throw std::invalid_argument("Failed to decode instruction at: " + std::format("0x{:x}", addr));
        }
        if (false) {
            std::println(stderr, "0x{:X}:\t{}\t\t{}", insn[0].address, insn[0].mnemonic, insn[0].op_str);
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

    m_Insn = std::move(insn);
    std::vector<Successor> succs = (_x86.*_semantics.at(id))(this);
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

    if (optimizeLiftedFunction) {
        Optimizer(m_Function).run();
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

    std::vector<Type*> state_fields(18, _i64Ty); // 17 GPRs + GS
    state_fields.insert(state_fields.end(), 6, _i8Ty);
    m_StateTy = namedStruct(m_Context, "State", state_fields);

    Type* pointerTy = PointerType::getUnqual(m_Context);
    m_LiftedTy = FunctionType::get(m_Builder.getVoidTy(), { pointerTy, pointerTy }, false);
}

} // namespace lifter
