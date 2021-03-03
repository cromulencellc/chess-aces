BUILD_DIR ?= ./build
SRC_DIR ?= ./src
PRIV_DIR ?= ./priv

TARGET ?= ovington

CC := clang
CXX := clang++

CPPFLAGS ?= -MD
CFLAGS ?= -g -Wall -Werror
CXXFLAGS ?= -g -Wall -Werror -std=c++17 -fPIE -fPIC
LDFLAGS ?= -g -std=c++17 -lstdc++fs -fPIE -pie

SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(filter-out ./build/request_test.o,$(OBJS)) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
