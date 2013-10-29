## -*-makefile-*- ############################################################
#  Standalone Makefile for DCE UUID Library and Tools
#    - for building the UUID library without the rest of DCE
#
#  Jim Doyle, October 1998, Boston University
#
#
# Things that you need to steal after the build and install on your system:
#
#   headers:   uuid.h uuid.hh
#   libraries: libuuid.a libuuid.so 
#   binaries:  uuid
#
##############################################################################
include ../Makefile.inc

SYSTEM = $(shell uname)


ifeq ($(SYSTEM), SunOS)
  XNET = -lxnet
else
  XNET =
endif

OBJS   = uuid-c.o uuid-cxx.o uuidsys.o get_ether_addr.o
LIBS   = libuuid.a 
SO_LIB = libuuid.so

CFLAGS   += -D_REENTRANT -I. -DUUID_BUILD_STANDALONE 
CXXFLAGS =  -D_REENTRANT -I.  -DUUID_BUILD_STANDALONE

RESULT:= uuid

LDFLAGS =

ifneq ($(SYSTEM), Linux)
  LDFLAGS += -L/usr/lib -lsocket -lc -ldl
else
  CFLAGS   += -DLINUX
  CXXFLAGS += -DLINUX
endif

CC      = gcc
CCC     = g++
LD      = gcc
AR      = ar
ARFLAGS = rcv

all global_all: $(LIBS) $(RESULT)

uuid-c.o: uuid.c
	$(CC) $(CFLAGS) -o uuid-c.o -c uuid.c

uuid-cxx.o: uuid.cc
	$(CCC) $(CXXFLAGS) -o uuid-cxx.o -c uuid.cc

libuuid.a: $(OBJS)
	$(AR) $(ARFLAGS) libuuid.a $(OBJS)

libuuid.so: $(OBJS)
	$(LD) -shared -o libuuid.so $(OBJS)

uuidtool.o: uuidtool.cc uuid.hh
	$(CCC) $(CXXFLAGS) -o uuidtool.o -c uuidtool.cc

uuid: uuidtool.o $(LIBS)
	$(CCC) -o uuid uuidtool.o  -L. $(LDFLAGS) -Bstatic -luuid

clean:
	rm -f $(OBJS)
	rm -f $(LIBS)
	rm -f uuid uuidtool.o

install: $(RESULT)
	@if [ -n "$(dest)" ] ; then \
	  echo "Installing to $(dest)" ; \
	  $(INSTALL_DIR) $(DIR_FLAGS) $(dest) ; \
	  echo "Installing $(RESULT) in $(CF_BIN_DIR)" ; \
	  $(INSTALL_FILES) $(BIN_FLAGS) $(RESULT) $(CF_BIN_DIR) ; \
	else  \
	  echo "usage: gmake install dest=<dir> " ; \
	  false ; \
	fi ;

	@true

%:
	@true
