BUILD_DIR ?= ./build
SRC_DIR ?= ./src

TARGET ?= dorchester.bin

.PHONY: clean $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): 
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)
	cd $(SRC_DIR) && make
	mv $(SRC_DIR)/$(TARGET) $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
