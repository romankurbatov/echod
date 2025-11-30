CXXFLAGS := $(CXXFLAGS) -std=c++17 -Wall -Werror
DESTDIR ?= /usr/local

headers := $(wildcard *.h *.hpp)

objects := main.o config.o debug.o dispatcher.o \
	udp_server.o tcp_server.o client.o \
	command_executor.o client_registry.o

exe := echod
service := echod@.service

$(objects): %.o: %.cpp $(headers)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(exe): $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $^

all: $(exe)

clean:
	rm -f $(objects) $(exe)

install: $(exe) $(service)
	install -D $(exe) $(DESTDIR)/bin/$(exe)
	install -D $(service) $(DESTDIR)/lib/systemd/system/$(service)

uninstall:
	rm -f $(DESTDIR)/bin/$(exe) $(DESTDIR)/lib/systemd/system/$(service)

.DEFAULT_GOAL := all
.PHONY: all clean install uninstall
