


build:
	mkdir build
	cd build && cmake ../

format-check:
	find src -name '*.cpp' | xargs clang-format --dry-run --Werror --style=file --fallback-style="LLVM"
	find src -name '*.h' | xargs clang-format --dry-run --Werror --style=file --fallback-style="LLVM"

format-run:
	find src -name '*.cpp' | xargs clang-format -i --Werror --style=file --fallback-style="LLVM"
	find src -name '*.h' | xargs clang-format -i --Werror --style=file --fallback-style="LLVM"

zig-build:
	.\dependencies\toolset\zig\zig.exe build --cache-dir build/cache

zig-wasm-build:
	.\dependencies\toolset\zig\zig.exe build -Dtarget=wasm32-freestanding-musl


# CC="zig cc -target wasm32-freestanding-musl" CXX="zig c++ -target wasm32-freestanding-musl" AR="zig ar" RANLIB="zig ranlib" uname_S="Linux" uname_M="x86_64" C11_ATOMIC=yes USE_JEMALLOC=no USE_SYSTEMD=no

coverage:
	gcovr -r .. --html --html-details		\
		-o coverage_report.html -j 8 -vvv	\
		--gcov-exclude '.*dependencies/.*'	\
		-e '.*dependencies/.*'				\
		--exclude-directories dependencies/

build-release:
	conan install conanfile.txt --build=missing --profile:build=./conan/release --profile:host=./conan/release
	cmake --preset release
	cmake --build --preset release

build-development:
	conan install conanfile.txt --build=missing --profile:build=./conan/development --profile:host=./conan/development
	cmake --preset conan-relwithdebinfo
	cmake --build --preset conan-relwithdebinfo

build-development-win32:
	conan install conanfile.txt --build=missing --profile:build=./conan/development --profile:host=./conan/development
	cmake --preset conan-default
	cmake --build --preset development


build-debug:
	conan install conanfile.txt --build=missing --profile:build=./conan/debug --profile:host=./conan/debug
	cmake --preset conan-debug
	cmake --build --preset conan-debug


build-emacscripten:
	conan install conanfile.txt --build=missing --profile:build=./conan/debug --profile:host=./conan/emscripten
	cmake --preset conan-release -DNO_LLVM=1
	cmake --build --preset conan-release


# conan install . -pr:b default -pr:h emscripten-wasm-clang -s build_type=Release -if cmake-build-release -b missing
# source ./cmake-build-release/conanbuild.sh
# cmake -S . -B cmake-build-release -DCMAKE_TOOLCHAIN_FILE=cmake-build-release/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
# cmake --build cmake-build-release --config Release
