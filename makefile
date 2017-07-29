CXX=g++-6
CXXFLAGS=-std=gnu++14 -O3 -g3 -ggdb -Wall -fmessage-length=0 -m32
SIMPLE_IPC_DIR = $(realpath simple-ipc/src/include)
INCLUDES=-I$(SIMPLE_IPC_DIR)
CXXFLAGS += $(INCLUDES)
LDFLAGS=-m32
LDLIBS=-lpthread -lrt
SRC_DIR = src
OUT_NAME = cathook-ipc-server
OUT_DIR = bin
SOURCES = $(shell find $(SIMPLE_IPC_DIR) -name "*.cpp" -print)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
SRC_SUBDIRS=$(shell find $(SRC_DIR) -type d -print)

.PHONY: clean directories

all:
	mkdir -p $(OUT_DIR)
	$(MAKE) bin/server
	
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_DIR)/server: $(SRC_DIR)/server.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

#$(TARGET): $(OBJECTS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $(TARGET)

clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.d' -delete
	rm -rf ./bin