#                            Package   : omniIFR
# Makefile                   Created   : 2003/12/28
#                            Author    : Alex Tingle
#
#    Copyright (C) 2003 Alex Tingle.
#
#    This file is part of the omniIFR application.
#
#    omniIFR is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    omniIFR is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

all: exe lib

include config.mk

MODULES        := repository
MODINCLUDES    := $(patsubst %,-I%,$(MODULES))

DAEMON_EXE     := $(OMNIIFR)$(EXEEXT)
LIBRARY        := $(call SharedLibName,$(OMNIIFR))
LIBRARY_SONAME := $(call SharedLibSoName,$(OMNIIFR),$(PACKAGE_VERSION))

# each module will add to this
OBJ_FILES :=

# include the description for each module
include $(patsubst %,%/module.mk,$(MODULES))

# include the C include dependencies
include $(OBJ_FILES:.$(OBJEXT)=.dep)

INSTALLED_HEADER_FILES := \
 $(patsubst %,$(INSTALL_INCLUDE)/$(OMNIIFR)/%,$(H_FILES_LIB))

# link
.PHONY: exe
exe: $(DAEMON_EXE)
$(DAEMON_EXE): $(OBJ_FILES_LIB) $(OBJ_FILES_EXE) 
	$(call CxxBuildExecutable,$@,$^)

.PHONY: lib
lib: $(LIBRARY)
$(LIBRARY): $(OBJ_FILES_LIB)
	$(call CxxBuildShared,$@,$^ $(ARCHIVES),,$(LIBRARY_SONAME))


# install
.PHONY: install
install: all install_dirs $(INSTALLED_HEADER_FILES)
	$(INSTALL) $(DAEMON_EXE) $(INSTALL_SBIN)
	$(call InstallSharedLib,$(OMNIIFR),$(PACKAGE_VERSION),$(INSTALL_LIB))
	$(MAKE) -C backend install

.PHONY: install_dirs
install_dirs:
	$(INSTALL) -d $(INSTALL_SBIN)
	$(INSTALL) -d $(INSTALL_LIB)
	$(INSTALL) -d $(INSTALL_INCLUDE)/$(OMNIIFR)
	$(INSTALL) -d /var/lib/$(OMNIIFR) || true

$(INSTALLED_HEADER_FILES): $(INSTALL_INCLUDE)/$(OMNIIFR)/%: repository/% install_dirs
	$(INSTALL) -m 644 $< $(INSTALL_INCLUDE)/$(OMNIIFR)


# uninstall
.PHONY: uninstall
uninstall:
	$(RMDIR) $(INSTALL_INCLUDE)/$(OMNIIFR)
	$(RMDIR) /var/lib/$(OMNIIFR)
	$(RM) $(INSTALL_SBIN)/$(DAEMON_EXE)
	$(MAKE) -C backend uninstall
	# Don't uninstall shared library.


# clean
.PHONY: clean
clean: 
	$(RM) $(DAEMON_EXE)
	$(RM) $(LIBRARY)
	$(RM) $(patsubst %,%/*.$(OBJEXT),$(MODULES))

.PHONY: maintainer-clean
maintainer-clean: clean
	$(RMDIR) html
	$(RM) $(patsubst %,%/*.dep,$(MODULES))

# doc
.PHONY: doc doc-force
doc: html/index.html
html/index.html: doxygen.conf
	doxygen doxygen.conf

# dist
DIST_FILES := \
  $(wildcard auto/*) \
  backend/ifr.py \
  backend/Makefile \
  browser/ifrclt.py \
  browser/IfrConnection.h \
  browser/ifrconnection.py \
  browser/ifrtest.cc \
  BUGS \
  CHANGES \
  config.mk.in \
  configure \
  COPYING \
  CREDITS \
  doxygen.conf \
  example.txt \
  Makefile \
  patch \
  patch/omniORB-4.0.3-get_interface.patch \
  patch/omniORBpy-2.3-keywords.patch \
  patch/README \
  README \
  $(wildcard repository/*.cc) \
  $(wildcard repository/*.dep) \
  $(wildcard repository/*.h) \
  repository/config.h.in \
  repository/module.mk \
  repository/TODO.txt \

DIST_TARGETS := $(patsubst %,%.dummy,$(DIST_FILES))
DIST_DIR := $(notdir $(CURDIR))

.PHONY: dist $(DIST_TARGETS)
dist: ../$(DIST_DIR).tar.gz
../$(DIST_DIR).tar.gz: ../$(DIST_DIR).tar
	gzip ../$(DIST_DIR).tar

../$(DIST_DIR).tar: $(DIST_TARGETS)
$(DIST_TARGETS): %.dummy: %
	cd .. && tar rf $(DIST_DIR).tar $(DIST_DIR)/$<
