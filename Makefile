CXX = g++
CXXFLAGS = -std=c++20 -Wall -Iinclude
TARGET = bin/flowhook
SRCS = src/main.cpp src/filewatcher.cpp src/display.cpp src/session_logger.cpp src/task_watcher.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)