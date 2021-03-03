BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= remsen

CC := clang++-7

CPPFLAGS ?= -MD -fpie -std=c++17
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -g -fpie -pie -std=c++17 -lstdc++fs

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

SRCS := $(HARDCODED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS_NTB := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o.ntb)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	cloc $(SRC_DIR)/*.cpp
	pmccabe -vtF $(SRC_DIR)/*.cpp
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	$(CC) $(OBJS_NTB) -o $@.ntb $(LDFLAGS) -DNO_TESTBED

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	$(CC) $(CPPFLAGS) $(CFLAGS) -DNO_TESTBED -c $< -o $@.ntb

.PHONY: clean docker

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

docker:
	docker build -t remsen .
	docker run --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt remsen

-include $(DEPS)
