CC := g++
CFLAGS := -Wall

BUILD_DIR := build/
SRC_DIR := src/
INCLUDE := -I$(SRC_DIR)/include
TARGET := $(BUILD_DIR)runner

SRCS=$(wildcard $(SRC_DIR)*.cpp)
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS:.cpp=.o)))

all: build

run: build
	@$(TARGET)

build: $(TARGET)

# Links all the object files after the .o files have been compiled.
$(TARGET): $(OBJS)
	$(CC) -o $@ $^

# Builds each individual .cpp file into a .o file.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDE)

# Creates the necessary build directories where executable is output.
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run build clean