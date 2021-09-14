# Include configure options
ifneq (clean,$(filter clean,$(MAKECMDGOALS)))
-include config.mk
endif

# sxiv version
VERSION = 26

# PREFIX for install
PREFIX ?= /usr/local
MANPREFIX = $(PREFIX)/share/man

# autoreload backend: inotify/nop
AUTORELOAD = inotify

# CFLAGS, any optimization flags goes here
CFLAGS ?= -std=c99 -Wall -pedantic

ifeq ($(HAVE_LIBEXIF), 1)
	OPTIONAL_LIBS += -lexif
else
	HAVE_LIBEXIF = 0
endif
ifeq ($(HAVE_LIBGIF), 1)
	OPTIONAL_LIBS += -lgif
else
	HAVE_LIBGIF = 0
endif

CPPFLAGS = -D_XOPEN_SOURCE=700 \
  -DHAVE_LIBGIF=$(HAVE_LIBGIF) -DHAVE_LIBEXIF=$(HAVE_LIBEXIF) \
  -I/usr/include/freetype2 -I$(PREFIX)/include/freetype2

LDLIBS = -lImlib2 -lX11 -lXft -lfontconfig $(OPTIONAL_LIBS)

OBJS = autoreload_$(AUTORELOAD).o commands.o image.o main.o options.o \
  thumbs.o util.o window.o

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

all: sxiv

sxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.c.o:
	@echo "CC $@"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJS): Makefile sxiv.h commands.lst config.h config.mk
options.o: version.h
window.o: icon/data.h

config.mk:
	@echo "GEN $@"
	@echo "# 0 = disable, 1 = enable" > config.mk
	@for lib in exif gif; do \
		if echo "int main(){}" | $(CC) "-l$$lib" -o /dev/null -x c - 2>/dev/null ; then \
			echo "HAVE_LIB$$lib=1" | tr '[:lower:]' '[:upper:]' >> config.mk ; \
		fi \
	done

config.h:
	@echo "GEN $@"
	cp config.def.h $@

version.h: Makefile .git/index
	@echo "GEN $@"
	v="$$(git describe 2>/dev/null)"; \
	echo "#define VERSION \"$${v:-$(VERSION)}\"" >$@

clean:
	$(RM) *.o sxiv

install: all
	@echo "INSTALL bin/sxiv"
	install -Dt $(DESTDIR)$(PREFIX)/bin sxiv
	@echo "INSTALL sxiv.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s!PREFIX!$(PREFIX)!g; s!VERSION!$(VERSION)!g" sxiv.1 \
		>$(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	@echo "INSTALL share/sxiv/"
	install -Dt $(DESTDIR)$(PREFIX)/share/sxiv/exec exec/*

uninstall:
	@echo "REMOVE bin/sxiv"
	rm -f $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "REMOVE sxiv.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	@echo "REMOVE share/sxiv/"
	rm -rf $(DESTDIR)$(PREFIX)/share/sxiv

