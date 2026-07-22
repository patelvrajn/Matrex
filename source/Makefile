# To compile using all available cores use: -j$(shell nproc)
# Only compile using 1 core because of memory consumption.
MAKEFLAGS += -j1

ifndef DEBUG
BUILD_DIRECTORY := ../build/source/release
else
BUILD_DIRECTORY := ../build/source/debug
endif

# Recursive wildcard make function; recursively searches for files matching 
# given wildcard pattern.
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Source files (*.cpp)
srcs = $(call rwildcard,.,*.cpp)

# Object files (*.o)
objs = $(patsubst ./%.cpp,$(BUILD_DIRECTORY)/%.o,$(srcs))

# Dependency information files (*.d)
depends = $(patsubst ./%.cpp,$(BUILD_DIRECTORY)/%.d,$(srcs))

# Name of project
PROJECT_NAME = matrex

# Compiler and compiler flags.
CXX      = g++
CXXFLAGS = \
		    -g \
		    -std=gnu++23 \
			-fconstexpr-depth=8192 \
			-march=native \
	        -O3 \
			-flto \
	        -MMD \
	        -MP \
	        -Wall \
	        -Wextra \
	        -Wpedantic \
			-Wno-interference-size \
		    -fpie \
	        -pie \
		    -I ../include

.PHONY: all clean

all: $(BUILD_DIRECTORY)/$(PROJECT_NAME)

# Clean compile outputs from last make.
clean:
	find ../build/source -name "*.d" -type f -delete
	find ../build/source -name "*.o" -type f -delete
	find ../build/source -name "$(PROJECT_NAME)" -type f -delete

# The final product depends on the object files.
$(BUILD_DIRECTORY)/$(PROJECT_NAME) : $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lstdc++exp

# An object file depends on the respective cpp file.
ifndef DEBUG
$(BUILD_DIRECTORY)/%.o: %.cpp
	$(CXX) -DNDEBUG $(CXXFLAGS) $< -o $@ -c
else
$(BUILD_DIRECTORY)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c
endif

# Include dependency rules output from previous make.
-include $(depends)
