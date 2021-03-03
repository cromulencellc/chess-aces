BUILD_DIR ?= ./build
SRC_DIR ?= ./src
PRIV_DIR ?= ./priv

TARGET ?= channing

#CC := afl-clang
CC := clang-7

CPPFLAGS ?= -MD
CFLAGS ?=  -g -Wall -Werror
LDFLAGS ?= -g -lm -lcrypt

HARDCODED_SRCS = $(wildcard $(SRC_DIR)/*.c)
#PARSER_GENERATED_SRCS = $(SRC_DIR)/parse.tab.c $(SRC_DIR)/parse.yy.c

SRCS := $(HARDCODED_SRCS) $(PARSER_GENERATED_SRCS)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.yy.c: $(PRIV_DIR)/%.lex $(SRC_DIR)/%.tab.c
	flex -o $@ $<

$(SRC_DIR)/%.tab.c: $(PRIV_DIR)/%.y
	bison -d -o $@ $<

.PHONY: clean docker

run: $(BUILD_DIR)/$(TARGET)
	$<

clean:
	rm -rf $(BUILD_DIR) $(PARSER_GENERATED_SRCS)

docker:
	docker build -t channing .
	docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --rm -it -v `pwd`:/mnt channing

-include $(DEPS)
