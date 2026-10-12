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

    void run() const;

private:
    llvm::Function* m_Function;
    bool ExhaustOptimizations{};
};

