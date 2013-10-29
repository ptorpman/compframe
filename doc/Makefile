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
include ../src/Makefile.inc

RESULT_HTML = compframe.html
RESULT_PDF  = compframe.pdf

HTML_FILES = $(shell ls /tmp/doxy/html/*.html | grep -v _source)

# HTML_FILES = 					\
# 	/tmp/doxy/html/index.html		\
# 	/tmp/doxy/html/group__Public.html \
# 	/tmp/doxy/html/group__Interfaces.html \
# 	/tmp/doxy/html/IBase_8hh.html

all:
	-rm /tmp/doxy/html/*.html
	doxygen compframe.doxy
	htmldoc -t pdf --titlefile title.html $(HTML_FILES) -f $(RESULT_PDF)


clean:
	-rm $(RESULT_PDF) $(RESULT_HTML) 
	-rm -rf html latex
