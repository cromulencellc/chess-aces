OBJS=\
	src/assembler/lexer.o \
	src/assembler/assembler.o \
	src/container/aa_tree.o \
	src/container/bb.o \
	src/container/binary.o \
	src/container/list.o \
	src/container/stack.o \
	src/error.o \
	src/instructions.o \
	src/runtime/jit_translate.o \
	src/runtime/jit.o \
	src/runtime/target_x86.o \
	src/rust.o \
        src/testbed.o

TARGET = ./verona

CC := clang-7

CFLAGS=-Wall -O3 -Werror
LIB=
INCLUDE=-I./src/

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) src/main.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)

run: $(TARGET)
	$(TARGET)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
	rm -rf container/*.o
	rm -rf assembler/*.o
	rm -rf runtime/*.o
	rm -rf *dSYM
	rm -f jitter-bugs
