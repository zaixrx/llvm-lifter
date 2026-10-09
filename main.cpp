#include "semantics.hpp"

#include <set>
#include <queue>
#include <print>
#include <fstream>

#include <llvm/TargetParser/Triple.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>

int main() {
    try {
        std::array<uint8_t, 0x9f> code{};
        std::ifstream file("example.o", std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open example.o");
        }
        if (!file.seekg(0x180, std::ios::beg)) {
            throw std::runtime_error("Failed to seek to offset 0x180 in example.o");
        }
        if (!file.read(reinterpret_cast<char*>(code.data()), code.size())) {
            throw std::runtime_error("Failed to read example.o");
        }

        llvm::LLVMContext context;
        llvm::Module _module("x86_mov_example", context);

        const llvm::Triple target(_module.getTargetTriple());
        _module.setTargetTriple(llvm::Triple("x86_64-pc-linux-gnu"));
        _module.setDataLayout("e-p:64:64-i64:64-i128:128-n8:16:32:64-S128");

        lifter::Semantics semantics(_module);

        std::queue<lifter::Successor> queue{};
        std::set<llvm::Value*> visited{};

        auto u64Ty = llvm::IntegerType::get(context, 64);
        queue.push({ 0, llvm::ConstantInt::get(u64Ty, 0) });

        while (!queue.empty()) {
            auto curr = queue.front();
            queue.pop();

            if (visited.contains(curr.dst)) {
                continue;
            }
            visited.insert(curr.dst);

            llvm::ConstantInt* cte = llvm::dyn_cast<llvm::ConstantInt>(curr.dst);
            if (!cte) {
                std::println("non-constant branch {:02X} ->", curr.src);
                curr.dst->print(llvm::outs());
                continue;
            }

            size_t insn_addr_start = cte->getZExtValue();
            if (insn_addr_start >= code.size()) {
                std::println("out branch to instruction @{:02X}", insn_addr_start);
                curr.dst->print(llvm::outs());
                continue;
            }
            size_t insn_addr_count = std::min({ code.size(), insn_addr_start + 15 }) - insn_addr_start;

            auto insn = std::span(code).subspan(insn_addr_start, insn_addr_count);
            auto next = semantics.liftBytes(cte->getZExtValue(), insn);
            for (auto succ : next) {
                if (visited.contains(succ.dst)) {
                    continue;
                }
                queue.push(succ);
            }
        }

        _module.print(llvm::outs(), nullptr);

        return 0;
    } catch (const std::exception& error) {
        llvm::errs() << "error: " << error.what() << '\n';
        return 1;
    }
}
