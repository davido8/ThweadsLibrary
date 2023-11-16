CC := g++
CFLAGS := -Wall

BUILD_DIR := build/
SRC_DIR := src/
INCLUDE := -I$(SRC_DIR)/include
TARGET := $(BUILD_DIR)runner
OBJ := $(BUILD_DIR)/Manager.o $(BUILD_DIR)/Thread.o $(BUILD_DIR)/Runner.o

all: build

run: build
	@$(TARGET)

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run build clean