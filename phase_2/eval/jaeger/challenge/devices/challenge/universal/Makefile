BUILD_PATH=../../build
OBJ_PATH=$(BUILD_PATH)/o
OBJS=\
	$(OBJ_PATH)/access_token.o \
	$(OBJ_PATH)/debug.o

TARGET = $(BUILD_PATH)/libchallenge.a

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
INCLUDE=-I../ -I ../../

$(TARGET) : $(OBJS)
	$(AR) r $(TARGET) $(OBJS)

$(OBJ_PATH)/%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f $(OBJ_PATH)/*.o
	rm -f $(TARGET)