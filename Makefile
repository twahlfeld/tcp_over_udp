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

sender: tcp.o ftp.o sender.o packet.o

receiver: tcp.o ftp.o receiver.o packet.o

ftp.o: ftp.h ftp.cpp tcp.cpp tcp.h

tcp.o: packet.o tcp.cpp tcp.h

packet.o: packet.cpp packet.h

.PHONY: clean
clean:
	rm -f *~ a.out core *.o sender receiver

.PHONY: all
all: clean default
