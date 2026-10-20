# Building

```sh
git clone --depth=1 https://github.com/zaixrx/llvm-lifter.git
cd llvm-lifter

cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Lift a raw x86-64 binary and optimize after instruction discovery:

```sh
./build/llvm-lifter example/blog_example.o > lifted.ll
./build/llvm-lifter code.bin --remove-state --remove-memory --exhaust > native.ll
```

`--remove-state` recovers an `i64` RAX return and scalar inputs in System V
integer argument order (RDI, RSI, RDX, RCX, R8, R9). Unused trailing argument
registers are omitted. Other live incoming registers or flags follow these
arguments explicitly; this includes RSP when the function accesses its stack.
The memory pointer remains the final argument until `--remove-memory` is used.
Other final register and flag values are discarded by native recovery.
Calls, indirect jumps, incomplete functions, escaping state pointers, and
functions with existing LLVM callers prevent state recovery.

`--remove-memory` replaces the guest memory base with native address accesses
and removes its argument. Guest addresses must be valid addresses in the current
process. The two passes can be enabled independently. `--exhaust` selects LLVM O3
instead of the default O1 simplification pipeline.

Library users run the passes after lifting is complete:

```cpp
auto* recovered = Optimizer(function)
    .stateRemoverPass(true)
    .memoryRemoverPass(true)
    .exhaustOptimizations(true)
    .run();
```

Signature recovery replaces the function, so use the returned pointer. Leave
`liftBytes` optimization disabled (the default) while discovering instructions;
the lifter caches block and register pointers that LLVM optimization can remove.

The regression suite compares compiled recovered functions with execution of
their lifted state interfaces. On x86-64, CMake registers it when Python, Clang,
and NASM are available:

```sh
ctest --test-dir build --output-on-failure
```
