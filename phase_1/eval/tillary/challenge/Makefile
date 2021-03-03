BUILD_DIR ?= ./build
SRC_DIR ?= ./src
PRIV_DIR ?= ./priv

TARGET ?= tillary

CC := clang
CXX := clang++

CPPFLAGS ?= -MD
CFLAGS ?= -g -Wall -Werror
CXXFLAGS ?= -g -Wall -Werror -std=c++17 -DBACKTRACES -O3 -DNO_TESTBED
LDFLAGS ?= -g -std=c++17 -lstdc++fs -rdynamic -ldl -rtlib=compiler-rt -lgcc_s

SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(filter-out ./build/XXXXX.o,$(OBJS)) -o $@ $(LDFLAGS)

# $(BUILD_DIR)/urlfuzz: $(OBJS)
#	$(CXX) $(filter-out ./build/main.o,$(OBJS)) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run all

all: $(BUILD_DIR)/$(TARGET)

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
