BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= jackson

CC := clang++-7

CPPFLAGS ?= -MD -fpie
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -g -fpie -pie

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

SRCS := $(HARDCODED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS_NTB := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o.ntb)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
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
	docker build -t jackson .
	docker run --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt jackson

-include $(DEPS)
