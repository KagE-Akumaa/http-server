CXX = g++
CXXFLAGS = -Wall -std=c++17
INCLUDES = -Iinclude

SRC = src/main.cpp src/server.cpp
OUT = http_server

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SRC)

clean:
	rm -f $(OUT)
