MAKEFILE_PATH := $(abspath .)
AFL_PATH?=$(realpath $(MAKEFILE_PATH)/AFLplusplus)
AFL_CC:=$(AFL_PATH)/afl-clang-fast
CFLAGS+=-g -L.. -I../include -I$(AFL_PATH)/include -pthread -laflpp -lm -Wall -Wextra -Wshadow -Wno-variadic-macros -D_FORTIFY_SOURCE=2 -O3 #-fstack-protector-strong -fsanitize=address -fno-omit-frame-pointer 

$(info AFL_PATH is $(AFL_PATH))

all: clean target executor

lib:
	make -C ..

afl:
	git submodule init
	git submodule update
	make -C $(AFL_PATH)
	make -C $(AFL_PATH)/llvm_mode
	
target: afl ./target.c
	$(AFL_CC) -g target.c -o target

executor: lib ./executor.c
	$(CC) executor.c -o executor $(CFLAGS)

test: clean executor target
	mkdir ./in || true
	echo "AAAAAA" > ./in/a
	echo "BBBBBB" > ./in/b
	LD_LIBRARY_PATH=.. ./executor ./in 2 ./target
	rm -rf ./in
	rm -rf ./out-*
	rm -rf ./crashes-* 2>/dev/null || trues

llmp-main: lib ./llmp-main.c
	$(CC) llmp-main.c -o llmp-main $(CFLAGS)

clean:
	rm out ./executor ./target ./success ./in-mem 2>/dev/null || true
	rm -rf ./in 2>/dev/null	|| true
	rm -rf ./crashes-* 2>/dev/null || true
	rm -rf ./llmp-main || true
	rm -rf ./out-*

in-memory-fuzzer: lib afl in-memory-fuzzer.c
	clang in-memory-fuzzer.c ./libpng.a -g $(CFLAGS) -L.  -Iexecutors -Ifeedbacks -laflpp -o in-mem -lz -lm  $(AFL_PATH)/afl-llvm-rt.o
