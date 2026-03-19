# SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

RM = rm -rf
INSTALL ?= install

default: _default


MODE ?= release
FEATURES ?= stack
FLAGS ?= ASSERTIONS

RDESC_MODE := $(MODE)
RDESC_FEATURES := $(FEATURES)
RDESC_FLAGS := $(FLAGS)

include rdesc.mk

_default: $(RDESC) $(RDESC_SO)

PREFIX ?= /usr/local
install: $(RDESC) $(RDESC_SO)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib/
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/rdesc/

	$(INSTALL) -m 755 $(RDESC_SO) $(DESTDIR)$(PREFIX)/lib/
	$(INSTALL) -m 644 $(RDESC) $(DESTDIR)$(PREFIX)/lib/

	$(INSTALL) -m 644 $(RDESC_INCLUDE_DIR)/* $(DESTDIR)$(PREFIX)/include/rdesc/

clean:
	$(RM) $(rdesc_DIST_DIR) docs-autogen

docs:
	doxygen


.PHONY: default _default clean docs install
