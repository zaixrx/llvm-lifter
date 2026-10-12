# Building

evidently requires clang installed

```sh
git clone --depth=1 https://github.com/zaixrx/llvm-lifter.git
cd llvm-lifter

cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
