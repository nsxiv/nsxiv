.POSIX:

include config.mk

inc_fonts_0 =
inc_fonts_1 = -I/usr/include/freetype2 -I$(PREFIX)/include/freetype2
lib_fonts_0 =
lib_fonts_1 = -lXft -lfontconfig
lib_exif_0 =
lib_exif_1 = -lexif
lib_gif_0 =
lib_gif_1 = -lgif
lib_webp_0 =
lib_webp_1 = -lwebpdemux -lwebp

nsxiv_cppflags = -D_XOPEN_SOURCE=700 \
  -DHAVE_LIBGIF=$(HAVE_LIBGIF) -DHAVE_LIBEXIF=$(HAVE_LIBEXIF) \
  -DHAVE_LIBWEBP=$(HAVE_LIBWEBP) -DHAVE_LIBFONTS=$(HAVE_LIBFONTS) \
  -DHAVE_INOTIFY=$(HAVE_INOTIFY) $(inc_fonts_$(HAVE_LIBFONTS)) \
  $(CPPFLAGS)

nsxiv_ldlibs = -lImlib2 -lX11 \
  $(lib_exif_$(HAVE_LIBEXIF)) $(lib_gif_$(HAVE_LIBGIF)) \
  $(lib_webp_$(HAVE_LIBWEBP)) $(lib_fonts_$(HAVE_LIBFONTS)) \
  $(LDLIBS)

objs = autoreload.o commands.o image.o main.o options.o \
  thumbs.o util.o window.o

.SUFFIXES:
.SUFFIXES: .c .o

all: nsxiv

nsxiv: $(objs)
	@echo "LINK $@"
	$(CC) $(LDFLAGS) -o $@ $(objs) $(nsxiv_ldlibs)

.c.o:
	@echo "CC $@"
	$(CC) $(CFLAGS) $(nsxiv_cppflags) -c -o $@ $<

$(objs): Makefile config.mk nsxiv.h config.h commands.h
options.o: version.h optparse.h
window.o: icon/data.h utf8.h

config.h:
	@echo "GEN $@"
	cp config.def.h $@

version.h: config.mk .git/index
	@echo "GEN $@"
	v="$$(git describe 2>/dev/null || true)"; \
	echo "#define VERSION \"$${v:-$(VERSION)}\"" >$@

.git/index:

dump_cppflags:
	@echo $(nsxiv_cppflags)

clean:
	rm -f *.o nsxiv version.h

install-all: install install-desktop install-icon

install-desktop:
	@echo "INSTALL nsxiv.desktop"
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp etc/nsxiv.desktop $(DESTDIR)$(PREFIX)/share/applications

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
	sed "s!EGPREFIX!$(EGPREFIX)!g; s!PREFIX!$(PREFIX)!g; s!VERSION!$(VERSION)!g" \
		etc/nsxiv.1 >$(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/nsxiv.1
	@echo "INSTALL share/nsxiv/"
	mkdir -p $(DESTDIR)$(EGPREFIX)
	cp etc/examples/* $(DESTDIR)$(EGPREFIX)
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

