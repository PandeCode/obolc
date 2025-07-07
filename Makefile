# Config
BUILD_DIR ?= build
BIN_NAME ?= obolc

.PHONY: all run release debug clean

all: $(BUILD_DIR)/Makefile
	@cmake --build $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

run: all
	@./$(BUILD_DIR)/$(BIN_NAME)

debug:
	@cmake -S . -B $(BUILD_DIR)-debug -DCMAKE_BUILD_TYPE=Debug
	@cmake --build $(BUILD_DIR)-debug

release:
	@cmake -S . -B $(BUILD_DIR)-release -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR)-release

clean:
	@rm -rf build build-debug build-release assets/style.css
