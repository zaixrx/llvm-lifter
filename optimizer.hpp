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

    // Signature recovery can replace the function. Use the returned pointer.
    llvm::Function* run();

private:
    llvm::Function* m_Function;
    bool EnableExhaustOptimizations{};
    bool EnableStateRemoverPass{};
    bool EnableMemoryRemoverPass{};
};

