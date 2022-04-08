BUILD_PATH=../build
PLATFORM_PATH=../platform

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
LIB=$(BUILD_PATH)/libplatform.a
LINK=-T ../link.ld
INCLUDE=-I../

all : $(BUILD_PATH)/hello-world-timer

$(BUILD_PATH)/hello-world-timer :
	${CC} main.c -o $(BUILD_PATH)/hello-world-timer $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

clean :
	rm -f $(BUILD_PATH)/hello-world-timer