# Include configure options
ifneq (clean,$(filter clean,$(MAKECMDGOALS)))
-include config.mk
endif

# nsxiv version
VERSION = 27.1

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

SRCS = autoreload_$(AUTORELOAD) commands image main options \
  thumbs util window

OBJS = obj/autoreload_$(AUTORELOAD).o obj/commands.o obj/image.o obj/main.o obj/options.o \
  obj/thumbs.o obj/util.o obj/window.o

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

all: nsxiv

nsxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(SRCS):
	@echo "CC src/$@.c"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o obj/$@.o src/$@.c

$(OBJS): Makefile src/nsxiv.h src/commands.lst config.h config.mk $(SRCS)
obj/options.o: src/version.h
obj/window.o: icon/data.h

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

src/version.h: Makefile .git/index
	@echo "GEN $@"
	v="$$(git describe 2>/dev/null)"; \
	echo "#define VERSION \"$${v:-$(VERSION)}\"" >$@

.git/index:

clean:
	$(RM) obj/*.o nsxiv

install: all
	@echo "INSTALL bin/nsxiv"
	install -Dt $(DESTDIR)$(PREFIX)/bin nsxiv
	@echo "INSTALL nsxiv.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s!PREFIX!$(PREFIX)!g; s!VERSION!$(VERSION)!g" nsxiv.1 \
		>$(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "INSTALL share/nsxiv/"
	install -Dt $(DESTDIR)$(PREFIX)/share/nsxiv/exec exec/*

uninstall:
	@echo "REMOVE bin/nsxiv"
	rm -f $(DESTDIR)$(PREFIX)/bin/nsxiv
	@echo "REMOVE nsxiv.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "REMOVE share/nsxiv/"
	rm -rf $(DESTDIR)$(PREFIX)/share/nsxiv

