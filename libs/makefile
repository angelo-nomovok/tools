CPP=g++

PROJECT=nutil

PROJDIR=$(CURDIR)
INCDIR=-I$(PROJDIR)/include

SRCDIR=$(PROJDIR)/src
OBJDIR=$(PROJDIR)/obj
BINDIR=$(PROJDIR)
CXXFLAGS=-std=c++11 -c -Wall -W -O3 -fno-strict-aliasing -pipe $(INCDIR)

LIBA=$(BINDIR)/lib$(PROJECT).a
LIBSO=$(BINDIR)/lib$(PROJECT).so

LIBDIR=
LIBS=

SRCS:=$(wildcard $(SRCDIR)/*.cc)
OBJS:=$(patsubst %.cc,%.o,$(SRCS))
OBJS:=$(patsubst $(SRCDIR)%,$(OBJDIR)%,$(OBJS))

all: $(LIBA) $(LIBSO)

static: $(LIBA)
dynamic: $(LIBSO)

$(LIBA): $(OBJS)
	ar rcs $(LIBA) $(OBJS)

$(LIBSO): $(OBJS)
	g++ -shared -o $(LIBSO) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CPP) $(CXXFLAGS) -fPIC $< -o $@

clean:
	rm -f obj/*
	rm -f libnutil*
