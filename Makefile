# Include configure options if configure was run
-include config.mk

version = 26

PREFIX ?= /usr/local
MANPREFIX = $(PREFIX)/share/man

# autoreload backend: inotify/nop
AUTORELOAD = inotify

ifeq ($(HAVE_LIBEXIF), 1)
	OPTIONAL_LIBS += -lexif
endif
ifeq ($(HAVE_LIBGIF), 1)
	OPTIONAL_LIBS += -lgif
endif

CFLAGS ?= -std=c99 -Wall -pedantic
CFLAGS += -D_XOPEN_SOURCE=700 \
  -DHAVE_LIBGIF=$(HAVE_LIBGIF) -DHAVE_LIBEXIF=$(HAVE_LIBEXIF) \
  -I/usr/include/freetype2 -I$(PREFIX)/include/freetype2

LDLIBS = -lImlib2 -lX11 -lXft -lfontconfig $(OPTIONAL_LIBS)

OBJS = autoreload_$(AUTORELOAD).o commands.o image.o main.o options.o \
  thumbs.o util.o window.o

all: config.mk sxiv

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

config.mk:
	:> config.mk
	for lib in exif gif; do \
		if echo "int main(){}" | $(CC) "-l$$lib" -o /dev/null -x c - 2>/dev/null ; then \
			echo "HAVE_LIB$$lib=1" | tr '[:lower:]' '[:upper:]' >> config.mk ; \
		fi \
	done

sxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(OBJS): Makefile sxiv.h commands.lst config.h config.mk
options.o: version.h
window.o: icon/data.h


config.h:
	@echo "GEN $@"
	cp config.def.h $@

version.h: Makefile .git/index
	@echo "GEN $@"
	v="$$(git describe 2>/dev/null)"; \
	echo "#define VERSION \"$${v:-$(version)}\"" >$@

.git/index:

clean:
	$(RM) *.o sxiv

install: all
	@echo "INSTALL bin/sxiv"
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp sxiv $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "INSTALL sxiv.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s!PREFIX!$(PREFIX)!g; s!VERSION!$(version)!g" sxiv.1 \
		>$(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	@echo "INSTALL share/sxiv/"
	mkdir -p $(DESTDIR)$(PREFIX)/share/sxiv/exec
	cp exec/* $(DESTDIR)$(PREFIX)/share/sxiv/exec/
	chmod 755 $(DESTDIR)$(PREFIX)/share/sxiv/exec/*

uninstall:
	@echo "REMOVE bin/sxiv"
	rm -f $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "REMOVE sxiv.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sxiv.1
	@echo "REMOVE share/sxiv/"
	rm -rf $(DESTDIR)$(PREFIX)/share/sxiv

