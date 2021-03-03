OBJS=\
	parser/o/ast.o \
	parser/o/parser.o \
	parser/o/token.o \
	parser/lexer.o \
	parser/parser.o \
	container/aa_tree.o \
	container/list.o \
	container/ou64.o \
	container/stack.o \
	container/test_item.o \
	container/vertex_identifier.c \
	graphdb/graph.o \
	graphdb/vertex_property.o \
	graphdb/vertex.o \
	runtime/interpreter.o \
	runtime/scope.o \
	object.o \
	rust.o 

TARGET = ./very-fast-graph-database

CFLAGS=-Wall -Werror -no-pie
LIB=
INCLUDE=-I./ -Iparser/ -I container/

$(TARGET) : $(OBJS)
	$(CC) -o challenge main.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)
	$(CC) -o test test.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)
	$(CC) -o test_suite test_suite.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)
	pmccabe -Tv \
		container/*.c \
		graphdb/*.c \
		parser/o/*.c \
		parser/*.c \
		runtime/*.c \
		*.c

run: $(TARGET)
	$(TARGET)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
	rm -f container/*.o
	rm -f parser/*.o
	rm -f parser/o/*.o
	rm -f graphdb/*.o
	rm -f runtime/*.o
	rm -rf *dSYM
	rm -f very-fast-graph-database