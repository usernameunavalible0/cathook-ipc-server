CXX=$(shell sh -c "which g++-6 || which g++")
CXXFLAGS=-std=gnu++14 -O3 -g3 -ggdb -Wall -fmessage-length=0 -m32
SIMPLE_IPC_DIR = $(realpath simple-ipc/src/include)
INCLUDES=-I$(SIMPLE_IPC_DIR)
CXXFLAGS += $(INCLUDES)
LDFLAGS=-m32
LDLIBS=-lpthread -lrt
SRC_DIR = src
OUT_NAME = cathook-ipc-server
OUT_DIR = bin
SOURCES = $(shell find $(SRC_DIR) -name "*.cpp" -print)
SOURCES += $(shell find $(SIMPLE_IPC_DIR) -name "*.cpp" -print)
DEPENDS = $(SOURCES:.cpp=.d)
TARGETS=server exec undeadlock exec_all console

SOURCES := $(filter-out $(patsubst %,src/%.cpp,$(TARGETS)),$(SOURCES))

OBJECTS = $(SOURCES:.cpp=.o)

$(info $(SOURCES))

.PHONY: clean directories

all:
	mkdir -p $(OUT_DIR)
	$(MAKE) $(addprefix bin/,$(TARGETS))

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_DIR)/%: $(SRC_DIR)/%.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.d' -delete
	rm -rf ./bin