


build:
	mkdir build
	cd build && cmake ../

format-check:
	find src -name '*.cpp' | xargs clang-format --dry-run --Werror --style=file --fallback-style="LLVM"
	find src -name '*.h' | xargs clang-format --dry-run --Werror --style=file --fallback-style="LLVM"

format-run:
	find src -name '*.cpp' | xargs clang-format -i --Werror --style=file --fallback-style="LLVM"
	find src -name '*.h' | xargs clang-format -i --Werror --style=file --fallback-style="LLVM"
