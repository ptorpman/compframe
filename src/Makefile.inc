#/****************************************************************************
# * Copyright (c) 2004 Peter R. Torpman.                                     *
# *                                                                          *
# * Permission is hereby granted, free of charge, to any person obtaining a  *
# * copy of this software and associated documentation files (the            *
# * "Software"), to deal in the Software without restriction, including      *
# * without limitation the rights to use, copy, modify, merge, publish,      *
# * distribute, distribute with modifications, sublicense, and/or sell       *
# * copies of the Software, and to permit persons to whom the Software is    *
# * furnished to do so, subject to the following conditions:                 *
# *                                                                          *
# * The above copyright notice and this permission notice shall be included  *
# * in all copies or substantial portions of the Software.                   *
# *                                                                          *
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
# * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
# * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
# * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
# * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
# * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
# *                                                                          *
# * Except as contained in this notice, the name(s) of the above copyright   *
# * holders shall not be used in advertising or otherwise to promote the     *
# * sale, use or other dealings in this Software without prior written       *
# * authorization.                                                           *
# ****************************************************************************/
#
#/****************************************************************************
# *  Author: Peter R. Torpman (peter at torpman dot se) 2006                 *
# ****************************************************************************/
CC       = gcc 

WARN_FLAGS = -Wall -Wextra -Werror -Wno-unused-but-set-variable
CFLAGS   = -std=gnu99 -g -O2 $(WARN_FLAGS)
CPPFLAGS = -I. -DTEST -DCF_VERSION='"0.5.2"'
LIBS     = -ldl 
CXXFLAGS = -g -O2  $(WARN_FLAGS)

INSTALL_DIR   =  install -d -m
INSTALL_FILES =  install -m
DIR_FLAGS     = 775
FILE_FLAGS    = 664
BIN_FLAGS     = 775

CF_BIN_DIR  = $(dest)/bin
CF_COMP_DIR = $(dest)/comp
CF_DOC_DIR  = $(dest)/doc

# W


# Update this to get to tcl.h
CPPFLAGS += -I/usr/include/tcl8.6
LIBS += -ltcl8.6

# ****************************************************************************/
# DEFAULT TARGETS 
# ****************************************************************************/
%.o: %.c
	@echo "COMPILING $^" ; $(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c $^
%.o: %.cc
	@echo "COMPILING $^" ; $(CC) $(CXXFLAGS) $(CPPFLAGS) -fPIC -c $^


