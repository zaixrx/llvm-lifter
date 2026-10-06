# Building

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

`example/semantics_test.cpp` is a runnable x86-64 test example:

```sh
./build/semantics-example 2>/tmp/semantics-trace.log
```

It tests MOV/MOVABS, AND, OR and XOR at 8/16/32/64 bits, using register,
immediate and memory operands. Byte register input pairs are exhaustive;
wider inputs use boundary values and repeatable random samples. It also checks
all GPR families, high-byte registers, partial writes, unchanged state/memory,
sign-extended immediates, unaligned accesses, SIB scales, RIP-relative addresses,
address-size wrapping, FS/GS bases, flag helpers, caches and rejected instructions.
ADD, SUB and MUL are deliberately unsupported.

Defined logical flags are compared with native x86 instructions. Undefined AF
uses the module's `i1 @__lifter_undef_af(i64 instruction_address)` hook. The test
defines an explicit policy returning the address's low bit so both possible
values are checked; this is not a prediction of hardware AF. A program executing
lifted logical instructions must supply its own implementation of that hook.
