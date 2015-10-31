CC  = clang++
CXX = clang++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++11

LDFLAGS = -g
LDLIBS  =

.PHONY: default
default: sender receiver
	rm -rf *~ a.out *.o *dSYM

sender: tcp.o sender.p

receiver: tcp.o receiver.o

tcp.o: tcp.cpp tcp.h

.PHONY: clean
clean:
	rm -f *~ a.out core $(objects) $(executables)

.PHONY: all
all: clean default
