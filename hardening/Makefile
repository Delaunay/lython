

fuzz: fuzz-build fuzz-run

fuzz-build:
	mkdir -p fuzzing/build

	#
	# Disable max recursion depth so we do not throw on far fetched examples
	#
	cd fuzzing/build && CC=/usr/bin/afl-clang-fast CXX=/usr/bin/afl-clang-fast++ cmake\
		-DCMAKE_BUILD_TYPE=RelWithDebInfo			\
		-DWITH_LOG=0								\
		-DWITH_COVERAGE=0							\
		-DLY_MAX_VISITOR_RECURSION_DEPTH=-1         \
		 ../../../

	cd fuzzing/build &&make -j 32

fuzz-run:
	rm -rf fuzzing/results
	afl-fuzz -m800 -i ./fuzzing/in -o fuzzing/results -- ./fuzzing/build/bin/lython internal --file @@


BAD_FILE = $(shell ls ./fuzzing/results/crashes | head -n 1)
HANG_FILE = $(shell ls ./fuzzing/results/hangs | head -n 1)

.PHONY: exec-crash exec-hangs

exec-crash:
	cd ../build && cmake -DWITH_LOG=1 -DCMAKE_BUILD_TYPE=Debug ../ && make lython -j 32

	echo ""
	cat "fuzzing/results/crashes/$(BAD_FILE)"
	echo ""

	../build/bin/lython internal --file "fuzzing/results/crashes/$(BAD_FILE)"

	echo "Crash fixed, removing"
	rm fuzzing/results/crashes/$(BAD_FILE)

exec-hangs:
	cd ../build && cmake -DWITH_LOG=1 -DCMAKE_BUILD_TYPE=Debug ../ && make lython -j 32

	echo ""
	cat "fuzzing/results/hangs/$(HANG_FILE)"
	echo ""

	../build/bin/lython internal --file "fuzzing/results/hangs/$(HANG_FILE)"

	echo "Hanged fixed, removing"
	rm fuzzing/results/hangs/$(HANG_FILE)



profile: profile-build profile-run

profile-build:
	mkdir -p profile/build

	cd profile/build && cmake				\
		-WITH_COVERAGE=0					\
		-DCMAKE_BUILD_TYPE=Release			\
		-DCMAKE_CXX_FLAGS=-pg				\
		-DCMAKE_SHARED_LINKER_FLAGS=-pg		\
		-DCMAKE_EXE_LINKER_FLAGS=-pg		\
		-DWITH_LOG=0						\
		-DWITH_COVERAGE=0					\
		../../../

	cd profile/build && make -j 32

profile-run:
	# generates gmon.out
	./profile/build/bin/lython internal --file coz/nothing.ly

	# print stuff
	gprof ./profile/build/bin/lython > profile.out


valgrind: valgrind-build valgrind-run

valgrind-build:
	# this disable mimalloc for valgrind builds because it does not play nice

	mkdir -p valgrind/build
	cd valgrind/build && cmake -DWITH_VALGRIND=1 -DWITH_COVERAGE=0 ../../../
	cd valgrind/build && make -j 32

valgrind-run:
	valgrind ./valgrind/build/bin/lython internal --file fuzzing/in/nothing.ly


valgrind-perf:
	# It provides all the information that Cachegrind does, plus extra information about callgraphs

	mkdir -p valgrind/build

	cd valgrind/build && cmake				\
		-DCMAKE_BUILD_TYPE=Release			\
		-DWITH_LOG=0						\
		../../../

	cd valgrind/build && make -j 32

	rm -rf callgrind.out.*
	valgrind --tool=callgrind ./valgrind/build/bin/lython internal --file fuzzing/in/nothing.ly


valgrind-cache:
	# Cachegrind is a cache profiler.
	# It performs detailed simulation of the I1, D1 and L2 caches in your CPU and so can accurately pinpoint the sources of cache misses in your code.
	# It identifies the number of cache misses, memory references and instructions executed for each line of source code,
	# with per-function, per-module and whole-program summaries.
	# It is useful with programs written in any language.
	# Cachegrind runs programs about 20--100x slower than normal.

	mkdir -p valgrind/build

	cd valgrind/build && cmake				\
		-DCMAKE_BUILD_TYPE=Release			\
		-DWITH_LOG=0						\
		../../../

	cd valgrind/build && make -j 32

	rm -rf cachegrind.out.*
	valgrind --tool=cachegrind ./valgrind/build/bin/lython internal --file fuzzing/in/nothing.ly


valgrind-heap:
	mkdir -p valgrind/build

	cd valgrind/build && cmake				\
		-DCMAKE_BUILD_TYPE=Release			\
		-DWITH_LOG=0						\
		../../../

	cd valgrind/build && make -j 32

	rm -rf massif.out.*
	valgrind --tool=massif ./valgrind/build/bin/lython internal --file fuzzing/in/nothing.ly


coz: coz-build coz-run

coz-build:
	mkdir -p coz/build

	# RelWithDebInfo does not work
	cd coz/build && cmake							\
		-WITH_COVERAGE=0							\
		-DCMAKE_BUILD_TYPE=Debug					\
		-DWITH_LOG=0								\
		-DWITH_COZ=1                                \
		-DWITH_COVERAGE=0							\
		../../../

	cd coz/build && make -j 32

coz-run:
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	coz run -s %/lython/src/% --- ./coz/build/bin/lython internal --file coz/nothing.ly
	echo "DONE"

coz-time: coz-build
	/usr/bin/time ./coz/build/bin/lython internal --file coz/nothing.ly > out.txt

