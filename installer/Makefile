PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
SCRIPTS := strata-guide setup-disks setup-mounts setup-tar strata-chroot strata-setup

all:
	@echo "installer scripts: $(SCRIPTS)"

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m755 $(SCRIPTS) $(DESTDIR)$(BINDIR)
	install -m644 common.sh $(DESTDIR)$(BINDIR)/common.sh

.PHONY: all install
