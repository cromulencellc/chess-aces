BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= garfield.bin

CC := g++

CFLAGS ?= 
LDFLAGS ?=

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(SRC_DIR)/parser.cpp $(SRC_DIR)/tokens.cpp

SRCS := $(HARDCODED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(SRC_DIR)/parser.cpp $(SRC_DIR)/tokens.cpp $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/parser.cpp:
	bison -d -o $(SRC_DIR)/parser.cpp $(SRC_DIR)/parser.y

$(SRC_DIR)/tokens.cpp:
	flex -o $(SRC_DIR)/tokens.cpp $(SRC_DIR)/tokens.l

.PHONY: clean $(SRC_DIR)/parser.cpp $(SRC_DIR)/tokens.cpp 

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm $(SRC_DIR)/parser.cpp
	rm $(SRC_DIR)/tokens.cpp
	rm -rf $(BUILD_DIR)

-include $(DEPS)
