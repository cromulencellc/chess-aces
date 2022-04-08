BUILD_PATH=../../build
PLATFORM_PATH=../../platform
CHALLENGE_PATH=../

SOURCE=acd2.c acd2_callbacks.c acd2_store.c acd2_stream.c

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
LIB=$(BUILD_PATH)/libplatform.a $(BUILD_PATH)/libchallenge.a
LINK=-T ../../link.ld
INCLUDE=-I../ -I../../

TARGET = $(BUILD_PATH)/acd2

$(TARGET) : $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)
	$(STRIP) $(TARGET)

clean :
	rm -f $(TARGET)