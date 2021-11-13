# nsxiv version
VERSION = 27.1

# PREFIX for install
PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man
EGPREFIX ?= $(PREFIX)/share/doc/nsxiv/examples

# default value for optional dependencies. 1 = enabled, 0 = disabled
OPT_DEP_DEFAULT ?= 1

# autoreload backend: 1 = inotify, 0 = none
HAVE_INOTIFY ?= $(OPT_DEP_DEFAULT)

# optional dependencies, see README for more info
HAVE_LIBFONTS ?= $(OPT_DEP_DEFAULT)
HAVE_LIBGIF ?= $(OPT_DEP_DEFAULT)
HAVE_LIBEXIF ?= $(OPT_DEP_DEFAULT)
HAVE_LIBWEBP ?= $(OPT_DEP_DEFAULT)

# CFLAGS, any optimization flags goes here
CFLAGS ?= -std=c99 -Wall -pedantic

# icons that will be installed via `make icon`
ICONS = 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png

inc_fonts_0 =
inc_fonts_1 = -I/usr/include/freetype2 -I$(PREFIX)/include/freetype2

CPPFLAGS = -D_XOPEN_SOURCE=700 \
  -DHAVE_LIBGIF=$(HAVE_LIBGIF) -DHAVE_LIBEXIF=$(HAVE_LIBEXIF) \
  -DHAVE_LIBWEBP=$(HAVE_LIBWEBP) -DHAVE_LIBFONTS=$(HAVE_LIBFONTS) \
  $(inc_fonts_$(HAVE_LIBFONTS))

lib_fonts_0 =
lib_fonts_1 = -lXft -lfontconfig
lib_exif_0 =
lib_exif_1 = -lexif
lib_gif_0 =
lib_gif_1 = -lgif
lib_webp_0 =
lib_webp_1 = -lwebpdemux -lwebp
autoreload_0 = nop
autoreload_1 = inotify
# using += because certain *BSD distros may need to add additional flags
LDLIBS += -lImlib2 -lX11 \
  $(lib_exif_$(HAVE_LIBEXIF)) $(lib_gif_$(HAVE_LIBGIF)) \
  $(lib_webp_$(HAVE_LIBWEBP)) $(lib_fonts_$(HAVE_LIBFONTS))

OBJS = autoreload_$(autoreload_$(HAVE_INOTIFY)).o commands.o image.o main.o options.o \
  thumbs.o util.o window.o

.PHONY: all clean install uninstall install-all install-icon uninstall-icon install-desktop
.SUFFIXES:
.SUFFIXES: .c .o

all: nsxiv

nsxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.c.o:
	@echo "CC $@"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJS): Makefile nsxiv.h config.h
options.o: version.h
window.o: icon/data.h

config.h:
	@echo "GEN $@"
	cp config.def.h $@

version.h: Makefile .git/index
	@echo "GEN $@"
	v="$$(git describe 2>/dev/null)"; \
	echo "#define VERSION \"$${v:-$(VERSION)}\"" >$@

.git/index:

clean:
	rm -f *.o nsxiv version.h

install-all: install install-desktop install-icon

install-desktop:
	@echo "INSTALL nsxiv.desktop"
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp nsxiv.desktop $(DESTDIR)$(PREFIX)/share/applications

install-icon:
	@echo "INSTALL icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		mkdir -p "$$dir"; \
		cp "icon/$$f" "$$dir/nsxiv.png"; \
		chmod 644 "$$dir/nsxiv.png"; \
	done

uninstall-icon:
	@echo "REMOVE icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		rm -f "$$dir/nsxiv.png"; \
	done

install: all
	@echo "INSTALL bin/nsxiv"
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp nsxiv $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/nsxiv
	@echo "INSTALL nsxiv.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s!EGPREFIX!$(EGPREFIX)!g; s!PREFIX!$(PREFIX)!g; s!VERSION!$(VERSION)!g" nsxiv.1 \
		>$(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "INSTALL share/nsxiv/"
	mkdir -p $(DESTDIR)$(EGPREFIX)
	cp examples/* $(DESTDIR)$(EGPREFIX)
	chmod 755 $(DESTDIR)$(EGPREFIX)/*

uninstall: uninstall-icon
	@echo "REMOVE bin/nsxiv"
	rm -f $(DESTDIR)$(PREFIX)/bin/nsxiv
	@echo "REMOVE nsxiv.1"
	rm -f $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "REMOVE nsxiv.desktop"
	rm -f $(DESTDIR)$(PREFIX)/share/applications/nsxiv.desktop
	@echo "REMOVE share/nsxiv/"
	rm -rf $(DESTDIR)$(EGPREFIX)

