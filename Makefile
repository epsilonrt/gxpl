###############################################################################
# (c) Copyright 2015 epsilonRT                                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = lib
CLEANER_SUBDIRS = tools examples test doc

# Chemin relatif du répertoire racine de xPL4Linux
PROJECT_ROOT = .

# Choix de l'architecture matérielle du système
ARCH = ARCH_GENERIC_LINUX
#ARCH = ARCH_ARM_RASPBERRYPI

# Enabling Debug information (ON / OFF)
#DEBUG = ON

#---------------- Install Options ----------------
prefix=/usr/local

INSTALL_BINDIR=$(prefix)/bin
INSTALL_DATDIR=$(prefix)/share
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS) $(CLEANER_SUBDIRS)
distclean: $(SUBDIRS) $(CLEANER_SUBDIRS) 
install: install_util $(SUBDIRS)
uninstall: $(SUBDIRS) uninstall_util

install_util: 
	$(MAKE) -w -C util $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH)

uninstall_util:
	$(MAKE) -w -C util $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH) DEBUG=$(DEBUG)

$(CLEANER_SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS) $(CLEANER_SUBDIRS)
