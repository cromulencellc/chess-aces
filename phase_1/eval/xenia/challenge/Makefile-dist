# -*- mode: makefile -*-

OBJS=\
	src/parser/o/ast.o \
	src/parser/o/parser.o \
	src/parser/o/token.o \
	src/parser/lexer.o \
	src/parser/parser.o \
	src/container/aa_tree.o \
	src/container/list.o \
	src/container/ou64.o \
	src/container/stack.o \
	src/container/test_item.o \
	src/container/vertex_identifier.c \
	src/graphdb/graph.o \
	src/graphdb/vertex_property.o \
	src/graphdb/vertex.o \
	src/runtime/interpreter.o \
	src/runtime/scope.o \
	src/object.o \
	src/rust.o \
	src/testbed.o

TARGET = ./xenia.bin

CFLAGS?=-Wall -Werror -fno-PIE
LIB=
INCLUDE=-I./src -I./src/parser/ -I./src/container/

CC=clang

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) src/main.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)

run: $(TARGET)
	$(TARGET)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f src/*.o
	rm -f src/container/*.o
	rm -f src/parser/*.o
	rm -f src/parser/o/*.o
	rm -f src/graphdb/*.o
	rm -f src/runtime/*.o
	rm -f $(TARGET)
