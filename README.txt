clang -emit-llvm ms.cpp -c -o ms.bc
clang++ -I/path/to/llvm/include -L/path/to/llvm/lib -lLLVM-10 CleanCut.cpp -o CleanCut
./CleanCut
