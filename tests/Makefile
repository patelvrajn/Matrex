# Recursive wildcard make function; recursively searches for files matching 
# given wildcard pattern.
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Source files (*.cpp)
prog_srcs_w_main = $(call rwildcard,../source,*.cpp)
prog_srcs = $(patsubst ../source/main.cpp,,$(prog_srcs_w_main)) # Remove prog's main function by removing main.cpp otherwise, multiple main definitions.
test_srcs = $(call rwildcard,./,*.cpp)

# Object files (*.o)
prog_objs = $(patsubst %.cpp,%.o,$(prog_srcs))  
test_objs = $(patsubst %.cpp,%.o,$(test_srcs))

# Dependency information files (*.d)
prog_depends = $(patsubst %.cpp,%.d,$(prog_srcs))
test_depends = $(patsubst %.cpp,%.d,$(test_srcs))

# Name of project
PROJECT_NAME = matrex

# Compiler and compiler flags.
CXX      = g++
CXXFLAGS = \
		      -g \
			    --coverage \
			    -fprofile-abs-path \
		      -std=c++23 \
		      -pthread \
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

all: $(PROJECT_NAME)

# Clean compile outputs from last make.
clean:
	find .. -name "*.d" -type f -delete
	find .. -name "*.o" -type f -delete
	find .. -name "*.gcno" -type f -delete
	find .. -name "*.gcda" -type f -delete
	find .. -name "*.gcov" -type f -delete
	find .. -name "$(PROJECT_NAME)" -type f -delete

# The final product depends on the object files.
$(PROJECT_NAME) : $(prog_objs) $(test_objs)
	$(CXX) $(CXXFLAGS) -o $@ $^ /usr/local/lib/libgtest.a

# An object file depends on the respective cpp file.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c

# Include dependency rules output from previous make.
-include $(prog_depends)
-include $(test_depends)
