#include "semantics.hpp"

namespace lifter {

namespace handlers {
auto HandleMov(Semantics& sem) -> std::vector<Successor>;
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
    };

    _semantics = {
        {X86_INS_MOV, handlers::HandleMov},
        {X86_INS_MOVABS, handlers::HandleMov},
    };
}

} // namespace lifter
