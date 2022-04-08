BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= garfield.bin

CC := g++

CFLAGS ?= 
LDFLAGS ?=

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

SRCS := $(HARDCODED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
