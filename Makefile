################################################################
# Builds all source files in the src directory
# It ignores all file paths that include an underscore
#
# debug - makes a debug build
# test - builds test files
# clean - removes build directory

SOURCE_DIR := src
BUILD_DIR := build
TEST_DIR := tests

COMPILER_SETTINGS := -Iinclude
LINKER_SETTINGS := -lcurses

#default#debug##################################################

SOURCES := $(shell find $(SOURCE_DIR) -name *.c -a ! -name _*.c)
OBJECTS := $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/prog: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LINKER_SETTINGS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(COMPILER_SETTINGS)

.PHONY: debug
debug: $(OBJECTS)
	$(CC) -g $(OBJECTS) -o $(BUILD_DIR)/debug $(LINKER_SETTINGS)

#test###########################################################

TEST_SOURCES := $(shell find $(TEST_DIR) -name *.c)
FILTERED_OBJECTS := $(filter-out %main.o,$(OBJECTS))

.PHONY: test
test: $(FILTERED_OBJECTS)
	$(foreach var,\
		$(TEST_SOURCES),\
		$(CC) \
			-g $(var) $(FILTERED_OBJECTS) \
			-o $(BUILD_DIR)/$(basename $(notdir $(var))) \
			$(COMPILER_SETTINGS) $(LINKER_SETTINGS))

#clean##########################################################

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	
