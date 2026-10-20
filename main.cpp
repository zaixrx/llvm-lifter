#include "semantics.hpp"
#include "optimizer.hpp"

#include <cstdint>
#include <set>
#include <queue>
#include <print>
#include <format>
#include <fstream>
#include <filesystem>

#include <llvm/TargetParser/Triple.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Verifier.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::println(stderr, "usage: {} code.x86 [--remove-state] [--remove-memory] [--exhaust]", argv[0]);
        return 1;
    }

    bool removeState = false, removeMemory = false, exhaust = false;
    for (int index = 2; index < argc; ++index) {
        const std::string_view option = argv[index];
        if (option == "--remove-state") removeState = true;
        else if (option == "--remove-memory") removeMemory = true;
        else if (option == "--exhaust") exhaust = true;
        else {
            std::println(stderr, "unknown option: {}", option);
            return 1;
        }
    }
    
    std::filesystem::path path = argv[1];
    if (!std::filesystem::exists(path)) {
        std::println(stderr, "file {} doesn't exist", argv[1]);
        return 1;
    }

    try {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error(std::format("failed to open {}", path.c_str()));
        }

        size_t file_size = std::filesystem::file_size(path);
        std::vector<uint8_t> code = std::vector<uint8_t>(file_size);
        if (!file.read(reinterpret_cast<char*>(code.data()), std::filesystem::file_size(path))) {
            throw std::runtime_error(std::format("failed to read {}", path.c_str()));
        }

        llvm::LLVMContext context;
        llvm::Module le_module(std::format("{}_lifted", path.filename().c_str()), context);
        le_module.setTargetTriple(llvm::Triple("x86_64-pc-linux-gnu"));
        le_module.setDataLayout("e-m:e-p:64:64-i64:64-i128:128-n8:16:32:64-S128");
        lifter::Semantics semantics(le_module);
        auto* function = semantics.begin(0);

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
                std::println(stderr, "non-constant branch {:02X} ->", curr.src);
                curr.dst->print(llvm::errs());
                llvm::errs() << '\n';
                continue;
            }

            size_t insn_addr_start = cte->getZExtValue();
            if (insn_addr_start >= code.size()) {
                std::println(stderr, "out branch to instruction @{:02X}", insn_addr_start);
                curr.dst->print(llvm::errs());
                llvm::errs() << '\n';
                continue;
            }
            size_t insn_addr_count = std::min({ code.size(), insn_addr_start + 15 }) - insn_addr_start;

            auto insn = std::span(code).subspan(insn_addr_start, insn_addr_count);
            auto next = semantics.liftBytes(cte->getZExtValue(), insn, false);
            for (auto succ : next) {
                if (visited.contains(succ.dst)) {
                    continue;
                }
                queue.push(succ);
            }
        }

        Optimizer(function)
            .exhaustOptimizations(exhaust)
            .stateRemoverPass(removeState)
            .memoryRemoverPass(removeMemory)
            .run();
        if (llvm::verifyModule(le_module, &llvm::errs())) {
            throw std::runtime_error("Failed to verify optimized module");
        }
        le_module.print(llvm::outs(), nullptr);

        return 0;
    } catch (const std::exception& error) {
        llvm::errs() << "error: " << error.what() << '\n';
        return 1;
    }
}
