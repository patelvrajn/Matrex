# To compile using all available cores use: -j$(shell nproc)
# Only compile using 2 cores because of memory consumption.
MAKEFLAGS += -j2

# Recursive wildcard make function; recursively searches for files matching 
# given wildcard pattern.
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Source files (*.cpp)
srcs = $(call rwildcard,./,*.cpp)

# Object files (*.o)
objs = $(patsubst %.cpp,%.o,$(srcs))

# Dependency information files (*.d)
depends = $(patsubst %.cpp,%.d,$(srcs))

# Name of project
PROJECT_NAME = matrex

# Compiler and compiler flags.
CXX      = g++
CXXFLAGS = \
		    -g \
		    -std=gnu++23 \
			-fconstexpr-depth=8192 \
	        -O3 \
			-flto \
	        -MMD \
	        -MP \
	        -Wall \
	        -Wextra \
	        -Wpedantic \
		    -fpie \
	        -pie \
		    -I ../include

.PHONY: all clean

ifndef DEBUG
all: $(PROJECT_NAME)
else
all: $(PROJECT_NAME)_debug
endif

# Clean compile outputs from last make.
clean:
	find .. -name "*.d" -type f -delete
	find .. -name "*.o" -type f -delete
	find .. -name "$(PROJECT_NAME)" -type f -delete
	find .. -name "$(PROJECT_NAME)_debug" -type f -delete

# The final product depends on the object files.
ifndef DEBUG
$(PROJECT_NAME) : $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lstdc++_libbacktrace
else
$(PROJECT_NAME)_debug : $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lstdc++_libbacktrace
endif

# An object file depends on the respective cpp file.
ifndef DEBUG
%.o: %.cpp
	$(CXX) -DNDEBUG $(CXXFLAGS) $< -o $@ -c
else
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c
endif

# Include dependency rules output from previous make.
-include $(depends)
