BUILD_DIR ?= ./build
SRC_DIR ?= ./src
PRIV_DIR ?= ./priv

TARGET ?= montague

CC := clang
CXX := clang++

CPPFLAGS ?= -MD
CFLAGS ?= -g -Wall -Werror
CXXFLAGS ?= -Wall -Werror -std=c++17 -O3 -fPIE -fPIC
LDFLAGS ?= -std=c++17 -lstdc++fs -fPIE -pie

SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(filter-out ./build/request_test.o,$(OBJS)) -o $@ $(LDFLAGS)
	strip --strip-unneeded $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/request_test: $(OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(filter-out ./build/main.o,$(OBJS)) -o $@ $(LDFLAGS)

.PHONY: clean docker run

request_test: $(BUILD_DIR)/request_test
	TEMPLATE_PATH=/mnt/base_data/templates/ $<

run: $(BUILD_DIR)/$(TARGET)
	TEMPLATE_PATH=/mnt/base_data/templates/ $<

clean:
	rm -rf $(BUILD_DIR)

docker:
	docker build -t $(TARGET) .
	docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt --name montague_base $(TARGET)

-include $(DEPS)
