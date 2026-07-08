# To compile using all available cores use: -j$(shell nproc)
# Only compile using 1 core because of memory consumption.
MAKEFLAGS += -j1

BUILD_DIRECTORY := ../build/tests

# Recursive wildcard make function; recursively searches for files matching 
# given wildcard pattern.
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Source files (*.cpp)
prog_srcs_w_main = $(call rwildcard,../source,*.cpp)
prog_srcs = $(patsubst ../source/main.cpp,,$(prog_srcs_w_main)) # Remove prog's main function by removing main.cpp otherwise, multiple main definitions.
test_srcs = $(call rwildcard,.,*.cpp)

# Object files (*.o)
prog_objs = $(patsubst ../source/%.cpp,$(BUILD_DIRECTORY)/%.o,$(prog_srcs))  
test_objs = $(patsubst ./%.cpp,$(BUILD_DIRECTORY)/%.o,$(test_srcs))

# Dependency information files (*.d)
prog_depends = $(patsubst ../source/%.cpp,$(BUILD_DIRECTORY)/%.d,$(prog_srcs))
test_depends = $(patsubst ./%.cpp,$(BUILD_DIRECTORY)/%.d,$(test_srcs))

# Name of project
PROJECT_NAME = matrex

# Compiler and compiler flags.
CXX      = g++
CXXFLAGS = \
		    -g \
		    -std=gnu++23 \
		    -fconstexpr-depth=8192 \
			-pthread \
			-march=native \
	        -O3 \
			-flto \
	        -MMD \
	        -MP \
	        -Wall \
	        -Wextra \
	        -Wpedantic \
		    -fpie \
	        -pie \
		    -I ../include \
		    -I /usr/local/include/gtest/

.PHONY: all clean

all: $(BUILD_DIRECTORY)/$(PROJECT_NAME)

# Clean compile outputs from last make.
clean:
	find ../build/tests -name "*.d" -type f -delete
	find ../build/tests -name "*.o" -type f -delete
	find ../build/tests -name "*.gcno" -type f -delete
	find ../build/tests -name "*.gcda" -type f -delete
	find ../build/tests -name "*.gcov" -type f -delete
	find ../build/tests -name "$(PROJECT_NAME)" -type f -delete

# The final product depends on the object files.
$(BUILD_DIRECTORY)/$(PROJECT_NAME) : $(prog_objs) $(test_objs)
	$(CXX) $(CXXFLAGS) -o $@ $^ /usr/local/lib/libgtest.a -lstdc++exp

# An object file depends on the respective cpp file.
$(BUILD_DIRECTORY)/%.o: ../source/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c

$(BUILD_DIRECTORY)/%.o: ./%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c

# Include dependency rules output from previous make.
-include $(prog_depends)
-include $(test_depends)
