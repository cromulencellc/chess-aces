BUILD_PATH=../build
PLATFORM_PATH=../platform

CFLAGS=-O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror
LIB=$(BUILD_PATH)/libplatform.a $(BUILD_PATH)/o/test-1.o
LINK=-T ../link.ld
INCLUDE=-I../

all : $(BUILD_PATH)/test-1-d0 $(BUILD_PATH)/test-1-d1 $(BUILD_PATH)/test-1-d2

$(BUILD_PATH)/o/test-1 : test-1.c
	${CC} -c test-1.c -o $(BUILD_PATH)/o/test-1.o $(INCLUDE) $(CFLAGS)

$(BUILD_PATH)/test-1-d0 : $(BUILD_PATH)/o/test-1
	${CC} test-1-d0.c -o $(BUILD_PATH)/test-1-d0 $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

$(BUILD_PATH)/test-1-d1 : $(BUILD_PATH)/o/test-1
	${CC} test-1-d1.c -o $(BUILD_PATH)/test-1-d1 $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

$(BUILD_PATH)/test-1-d2 : $(BUILD_PATH)/o/test-1
	${CC} test-1-d2.c -o $(BUILD_PATH)/test-1-d2 $(INCLUDE) $(CFLAGS) $(LIB) $(LINK)

clean :
	rm -f $(BUILD_PATH)/test-1-*