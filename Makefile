CXXFLAGS := $(CXXFLAGS) -std=c++17 -Wall -Werror
PREFIX ?= /usr/local

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

dst_exe_path := $(DESTDIR)$(PREFIX)/bin/$(exe)
dst_service_path := $(DESTDIR)$(PREFIX)/lib/systemd/system/$(service)

install: $(exe) $(service)
	install -D $(exe) $(dst_exe_path)
	install -D $(service) $(dst_service_path)

uninstall:
	rm -f $(dst_exe_path) $(dst_service_path)

.DEFAULT_GOAL := all
.PHONY: all clean install uninstall
