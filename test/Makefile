MAKEFILE_PATH := $(abspath .)
AFL_DIR:=$(realpath $(MAKEFILE_PATH)/../../)
AFL_CC:=$(AFL_DIR)/AFLplusplus/afl-clang
CFLAGS+=-g -L$(AFL_DIR)/LibAFL -I$(AFL_DIR)/LibAFL/include -fsanitize=address -I$(AFL_DIR)/include -laflpp -lm -Wall -Wextra -Werror -Wno-unused-parameter -fno-omit-frame-pointer -D_FORTIFY_SOURCE=2 -O1 -fstack-protector

all: unit_test test clean

unit_test.o: ./unit_test.c
	$(CC) ./unit_test.c -o ./unit_test -Wl,--wrap=exit -Wl,--wrap=printf -lcmocka $(CFLAGS)

unit_test:	unit_test.o
	$(CC) ./unit_test.c -o ./unit_test -Wl,--wrap=exit -Wl,--wrap=printf -lcmocka $(CFLAGS)

test: unit_test
	LD_LIBRARY_PATH=.. ./unit_test

clean:
	rm ./unit_test