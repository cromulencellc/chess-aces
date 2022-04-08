BUILD_PATH=../build
PLATFORM_PATH=../platform

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
LIB=$(BUILD_PATH)/libplatform.a
LINK=-T ../link.ld
INCLUDE=-I../

all : $(BUILD_PATH)/test-0-d0 $(BUILD_PATH)/test-0-d1

$(BUILD_PATH)/test-0-d0 :
	${CC} test-0-d0.c -o $(BUILD_PATH)/test-0-d0 $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

$(BUILD_PATH)/test-0-d1 :
	${CC} test-0-d1.c -o $(BUILD_PATH)/test-0-d1 $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

clean :
	rm -f $(BUILD_PATH)/test-0-*