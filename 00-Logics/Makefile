# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

# Executable name
TARGET = satisfiability-tt

# Source file
SRCS = satisfiability-tt.cpp

# Object file
OBJS = $(SRCS:.cpp=.o)

# Default target: build the executable
all: $(TARGET)

# Link object file(s) to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

