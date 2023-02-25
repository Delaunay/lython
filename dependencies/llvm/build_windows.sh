
$env:SOURCE_DIR="E:/llvm-project"
$env:BUILD_DIR="E:/llvm-build"
$env:INSTALL_DIR="E:/llvm-install"

cd $env:BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$env:INSTALL_DIR -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS=clang -G "Visual Studio 17 2022" -A x64 -Thost=x64 $env:SOURCE_DIR/llvm
cmake --build . 
cmake -DCMAKE_INSTALL_PREFIX="$env:INSTALL_DIR" -P cmake_install.cmake


$env:INSTALL_DIR="E:/work/lython/binaries/win64"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$env:INSTALL_DIR -G "Visual Studio 17 2022" ..
cmake --build . 
cmake -DCMAKE_INSTALL_PREFIX="$env:INSTALL_DIR" -P cmake_install.cmake
