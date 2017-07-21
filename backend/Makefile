#                            Package   : omniEvents
# backend/Makefile           Created   : 2004/03/20
#                            Author    : Alex Tingle
#
#    Copyright (C) 2004 Alex Tingle.
#
#    This file is part of the omniifr application.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
#    USA.
#

include ../config.mk

.PHONY: all
all: ifr.pyc

ifr.pyc: ifr.py
	$(PYTHON) -c "import compileall; compileall.compile_dir('.')"

.PHONY: install
install: all
	$(INSTALL) -d $(INSTALLPYTHONDIR)/omniidl_be
	$(INSTALL) ifr.py $(INSTALLPYTHONDIR)/omniidl_be
	$(INSTALL) ifr.pyc $(INSTALLPYTHONDIR)/omniidl_be

.PHONY: uninstall
uninstall:
	$(RM) $(INSTALLPYTHONDIR)/omniidl_be/ifr.py
	$(RM) $(INSTALLPYTHONDIR)/omniidl_be/ifr.pyc

.PHONY: clean
clean:
	$(RM) ifr.pyc
