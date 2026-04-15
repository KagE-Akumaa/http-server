CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./include

SRC = main.cpp $(wildcard src/*.cpp)

TARGET = server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
