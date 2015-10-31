CC  = gcc
CXX = g++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++11

LDFLAGS = -g
LDLIBS  =

.PHONY: default
default: sender receiver
	rm -rf *~ a.out *.o *dSYM

sender: tcp.o sender.o

receiver: tcp.o receiver.o

tcp.o: tcp.cpp tcp.h

.PHONY: clean
clean:
	rm -f *~ a.out core *.o sender receiver

.PHONY: all
all: clean default
