
$env:SOURCE_DIR="G:/llvm-project"
$env:BUILD_DIR="G:/llvm-build"
$env:INSTALL_DIR="G:/llvm-install"


# -DLLVM_TARGETS_TO_BUILD
# AArch64;AMDGPU;ARM;AVR;BPF;Hexagon;Lanai;LoongArch;Mips;MSP430;NVPTX;PowerPC;RISCV;Sparc;SystemZ;VE;WebAssembly;X86;XCore
#       X86
#       AMDGPU
#       NVPTX
#       WebAssembly
#
#       AArch64
#

$LLVM_ARGS = "-DCMAKE_INSTALL_PREFIX=${env:INSTALL_DIR} -DLLVM_ENABLE_PROJECTS=clang -A x64 -Thost=x64"
$TARGETS = "X86;AMDGPU;NVPTX;WebAssembly"

# 
cd $env:BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" $LLVM_ARGS -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . 

cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" $LLVM_ARGS -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . 

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -G "Visual Studio 17 2022" $LLVM_ARGS -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . 

cmake -DCMAKE_INSTALL_PREFIX="$env:INSTALL_DIR" -P cmake_install.cmake
