


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
