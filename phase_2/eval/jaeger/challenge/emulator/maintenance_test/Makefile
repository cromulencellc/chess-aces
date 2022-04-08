OBJS=

TARGET = ./maintenance_test

LIBEMULATOR_PATH=../lib

CC=c99

CFLAGS=-Wall -g -Werror -O2
LIB=$(LIBEMULATOR_PATH)/libemulator.a
INCLUDE=-I $(LIBEMULATOR_PATH)

$(TARGET) : $(OBJS) main.c
	$(CC) -o $(TARGET) main.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)

run: $(TARGET)
	$(TARGET)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f $(TARGET)