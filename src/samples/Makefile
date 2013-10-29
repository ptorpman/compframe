#  Copyright (c) 2007  Peter R. Torpman (peter at torpman dot se)
# 
#  This file is part of CompFrame (http://compframe.sourceforge.net)
# 
#  CompFrame is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
# 
#  CompFrame is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.or/licenses/>.
# 
include ../Makefile.inc

SAMPLE      := sample.so 
SAMPLE_SRC  := sample1.cc 
SAMPLE_OBJ  := $(SAMPLE_SRC:.cc=.o)
SAMPLE2     := sample2.so 
SAMPLE2_SRC := sample2.c 
SAMPLE2_OBJ := $(SAMPLE2_SRC:.c=.o)

SAMPLEM     := sample_m_client
SAMPLEM_SRC := sample_m_client.c
SAMPLEM_OBJ := $(SAMPLEM_SRC:.c=.o)

CPPFLAGS  += -I../ 
LDFLAGS += -L../  

# ****************************************************************************/
# MAIN TARGETS
# ****************************************************************************/
all: $(SAMPLE)   $(SAMPLEM)
#$(SAMPLE2)

$(SAMPLE): $(SAMPLE_OBJ)
	$(CC) -shared $^ -o $@
$(SAMPLE2): $(SAMPLE2_OBJ)
	$(CC) -shared $^ -o $@

$(SAMPLEM):  $(SAMPLEM_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) -lcompframe_m_client

install: $(SAMPLE) $(SAMPLE2) $(SAMPLEM)
	@if [ -n "$(dest)" ] ; then \
	  echo "Installing to $(dest)" ; \
	  $(INSTALL_DIR) $(DIR_FLAGS) $(dest) ; \
	  echo "Installing $(SAMPLE) in $(CF_COMP_DIR)" ; \
	  $(INSTALL_FILES) $(BIN_FLAGS) $(SAMPLE) $(CF_COMP_DIR) ; \
	  echo "Installing $(SAMPLE2) in $(dest)" ; \
	  $(INSTALL_FILES) $(BIN_FLAGS) $(SAMPLE2) $(CF_COMP_DIR) ; \
	  echo "Installing $(SAMPLEM) in $(CF_BIN_DIR)" ; \
	  $(INSTALL_FILES) $(BIN_FLAGS) $(SAMPLEM) $(CF_BIN_DIR) ; \
	else  \
	  echo "usage: gmake install dest=<dir> " ; \
	  false ; \
	fi ;

# ****************************************************************************/




clean:
	-rm *.o *.so $(SAMPLEM)
