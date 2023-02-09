# Builds all source files in the src directory
# It ignores all file paths that include an underscore
#
# debug - makes a debug build
# clean - removes build directory

SOURCE_DIR := src
BUILD_DIR := build

COMPILER_SETTINGS := -Iinclude
LINKER_SETTINGS := -lcurses

SOURCES := $(shell find $(SOURCE_DIR) -name *.c -a ! -name _*.c)
OBJECTS := $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/prog: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LINKER_SETTINGS)

.PHONY: debug
debug: $(OBJECTS)
	$(CC) -g $(OBJECTS) -o $@ $(LINKER_SETTINGS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(COMPILER_SETTINGS)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	
