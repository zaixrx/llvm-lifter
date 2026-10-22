#pragma once

namespace llvm
{
class Function;
class Module;
};

class Optimizer
{
public:
    explicit Optimizer(llvm::Function* Function) : m_Function(Function) {}

    Optimizer&& exhaustOptimizations(bool on);
    Optimizer&& stateRemoverPass(bool on);
    Optimizer&& memoryRemoverPass(bool on);

    llvm::Function* run();

private:
    llvm::Function* m_Function;
    bool EnableExhaustOptimizations{};
    bool EnableStateRemoverPass{};
    bool EnableMemoryRemoverPass{};
};

