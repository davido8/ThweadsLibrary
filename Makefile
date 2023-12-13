CC := g++
CFLAGS := -Wall -g -fPIC -pthread

BUILD_DIR := build/
SRC_DIR := src/
INCLUDE := -I$(SRC_DIR)/include
TARGET := $(BUILD_DIR)runner
DLL := $(BUILD_DIR)libdthreads.so

SRCS := $(wildcard $(SRC_DIR)*.cpp)
OBJS := $(BUILD_DIR)Runner.o $(BUILD_DIR)Tests.o 
DLL_OBJS := $(BUILD_DIR)DThreads.o $(BUILD_DIR)Synch.o $(BUILD_DIR)Thread.o

all: build run

run: build
	@echo "Run export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:build/ if can't find library!"
	@$(TARGET)

build: $(TARGET)

dll: $(DLL)

# Links all the object files after the .o files have been compiled.
$(TARGET): $(OBJS) $(DLL)
	$(CC) -o $@ $(OBJS) -L/u/davido/dthreads/build -ldthreads -pthread

$(DLL): $(DLL_OBJS)
	$(CC) -shared -o $@ -fPIC $^

# Builds each individual .cpp file into a .o file.
$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp | $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDE)

# Creates the necessary build directories where executable is output.
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run build clean
