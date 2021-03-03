.PHONY: clean

BUILD_DIR	?= ./build
SRC_DIR		?= ./src

TARGET		?= girard

CXXC		:= clang-7
CPPFLAGS	?=
CXXFLAGS	?= -std=c++11 -g -Wall -Wextra -Werror
LDFLAGS		?= -g -lstdc++

SRCS		:= $(wildcard $(SRC_DIR)/*.cxx)
OBJS		:= $(SRCS:$(SRC_DIR)/%.cxx=$(BUILD_DIR)/%.o)
OBJS_NTB    := $(SRCS:$(SRC_DIR)/%.cxx=$(BUILD_DIR)/%.o.ntb)

debug: CXXFLAGS += -DDEBUG
debug: all

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXXC) $(OBJS) -o $@ $(LDFLAGS)
	$(CXXC) $(OBJS_NTB) -o $@.ntb $(LDFLAGS) -DNO_TESTBED

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	mkdir -p $(dir $@)
	$(CXXC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	$(CXXC) $(CPPFLAGS) $(CXXFLAGS) -DNO_TESTBED -c $< -o $@.ntb

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

stats:
	find src -type f | xargs pmccabe -Tv
	cloc src

docker:
	docker build -t girard .
	docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt girard
