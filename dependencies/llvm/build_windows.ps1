
$env:SOURCE_DIR="K:/llvm-project"
$env:BUILD_DIR="K:/llvm-build"
$env:INSTALL_DIR="K:/llvm-install"


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
$OLD = "$pwd"

#
New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/Debug
cd $env:BUILD_DIR/Debug
cmake -G "Visual Studio 17 2022" $LLVM_ARGS -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . --config Debug

New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/Release
cd $env:BUILD_DIR/Release
cmake -G "Visual Studio 17 2022" $LLVM_ARGS -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . --config Release
cmake -DCMAKE_INSTALL_PREFIX="$env:INSTALL_DIR" -P cmake_install.cmake

New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/RelWithDebInfo
cd $env:BUILD_DIR/RelWithDebInfo
cmake  -G "Visual Studio 17 2022" $LLVM_ARGS -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TARGETS_TO_BUILD="$TARGETS" $env:SOURCE_DIR/llvm
cmake --build . --config RelWithDebInfo

cd $OLD

# New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/binaries/Debug
# New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/binaries/Release
# New-Item -ItemType Directory -Force -Path $env:BUILD_DIR/binaries/RelWithDebInfo

# Move-Item -Path "$env:BUILD_DIR/Debug/Debug" -Destination "$env:BUILD_DIR/binaries/Debug" -Recurse
# Move-Item -Path "$env:BUILD_DIR/Release/Release" -Destination "$env:BUILD_DIR/binaries/Release" -Recurse
# Move-Item -Path "$env:BUILD_DIR/RelWithDebInfo/RelWithDebInfo" -Destination "$env:BUILD_DIR/binaries/RelWithDebInfo" -Recurse
