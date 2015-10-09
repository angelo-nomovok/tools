BINARY=rtt

LIBPATH=../libs
INCLIB=$(LIBPATH)/include


all:
	g++ -I$(INCLIB) -L$(LIBPATH) -std=c++11 -o $(BINARY) main.cc -lnutil -lglog -lgflags -lpthread
