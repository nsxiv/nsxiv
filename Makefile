# Include configure options
include config.mk

ifeq ($(HAVE_LIBEXIF), 1)
	OPTIONAL_LIBS += -lexif
endif
ifeq ($(HAVE_LIBGIF), 1)
	OPTIONAL_LIBS += -lgif
endif

OBJS = autoreload_$(AUTORELOAD).o commands.o image.o main.o options.o \
  thumbs.o util.o window.o

all: sxiv

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

sxiv: $(OBJS)
	@echo "LINK $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(OBJS): Makefile sxiv.h commands.lst config.h
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
	$(RM) *.o sxiv

install: all
	@echo "INSTALL bin/sxiv"
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp sxiv $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/sxiv
	@echo "INSTALL sxiv.1"
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s!PREFIX!$(PREFIX)!g; s!VERSION!$(VERSION)!g" sxiv.1 \
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
