BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= arborway.bin

CC := clang++-7

override CPPFLAGS += -MD -fpie -std=c++17
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -g -fpie -pie -std=c++17 -lstdc++fs -lcapstone  -Wl,-strip-all

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

SRCS := $(HARDCODED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean docker

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

docker:
	docker build -t arborway .
	docker run --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt arborway

-include $(DEPS)
