CXXFLAGS := $(CXXFLAGS) -std=c++17 -Wall -Werror

headers := $(wildcard *.h *.hpp)
objects := main.o config.o debug.o dispatcher.o
exe := echod

$(objects): %.o: %.cpp $(headers)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(exe): $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $^

all: $(exe)

clean:
	rm -f $(objects) $(exe)

.DEFAULT_GOAL := all
.PHONY: all clean
