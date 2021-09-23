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

# icons that will be installed via `make icon`
ICONS = 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png

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

.PHONY: all clean install uninstall icon
.SUFFIXES:
.SUFFIXES: .c .o

all: nsxiv

nsxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.c.o:
	@echo "CC $@"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJS): Makefile nsxiv.h commands.lst config.h config.mk
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

.git/index:

clean:
	$(RM) *.o nsxiv

desktop:
	@echo "INSTALL nsxiv.desktop"
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp nsxiv.desktop $(DESTDIR)$(PREFIX)/share/applications

icon:
	@echo "INSTALL icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		mkdir -p "$$dir"; \
		cp "icon/$$f" "$$dir/nsxiv.png"; \
		chmod 644 "$$dir/nsxiv.png"; \
	done

icon_cleanup:
	@echo "REMOVE icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		rm -f "$$dir/nsxiv.png"; \
	done

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

uninstall: icon_cleanup
	@echo "REMOVE bin/nsxiv"
	rm -f $(DESTDIR)$(PREFIX)/bin/nsxiv
	@echo "REMOVE nsxiv.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "REMOVE nsxiv.desktop"
	rm -f $(DESTDIR)$(PREFIX)/share/applications/nsxiv.desktop
	@echo "REMOVE share/nsxiv/"
	rm -rf $(DESTDIR)$(PREFIX)/share/nsxiv

