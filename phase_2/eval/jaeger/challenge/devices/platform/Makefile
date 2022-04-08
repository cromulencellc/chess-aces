BUILD_PATH=../build
OBJ_PATH=$(BUILD_PATH)/o
OBJS=\
	$(OBJ_PATH)/platform.o \
	$(OBJ_PATH)/platform_asm.o \
	$(OBJ_PATH)/stdlib.o \
	$(OBJ_PATH)/prot.o

TARGET = $(BUILD_PATH)/libplatform.a

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
INCLUDE=-I./

$(TARGET) : $(OBJS)
	$(AR) r $(TARGET) $(OBJS)

$(OBJ_PATH)/%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

$(OBJ_PATH)/%.o : %.s
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f $(OBJ_PATH)/*.o
	rm -f $(TARGET)