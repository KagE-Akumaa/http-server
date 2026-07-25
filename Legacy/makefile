
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I.

# Source files (all .cpp files in current folder)
SRC = mainServer.cpp HTTP_SERVER.cpp URL_Util.cpp 

# Output executable
TARGET = server

# Default target
all: $(TARGET)

# Compile and link
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean build files
clean:
	rm -f $(TARGET)
