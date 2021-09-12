# sxiv version
VERSION = 26

PREFIX ?= /usr/local
MANPREFIX = $(PREFIX)/share/man

# autoreload backend: inotify/nop
AUTORELOAD = inotify

# enable features requiring giflib (-lgif)
HAVE_LIBGIF = 1

# enable features requiring libexif (-lexif)
HAVE_LIBEXIF = 1

# includes and flags
INCS= -I/usr/include/freetype2 -I$(PREFIX)/include/freetype2
CPPFLAGS = $(INCS) -D_XOPEN_SOURCE=700 \
  -DHAVE_LIBGIF=$(HAVE_LIBGIF) -DHAVE_LIBEXIF=$(HAVE_LIBEXIF) \

# CFLAGS
CFLAGS ?= -O2
CFLAGS += -std=c99 -Wall -pedantic

# libs
LDLIBS = -lImlib2 -lX11 -lXft -lfontconfig $(OPTIONAL_LIBS)

# compiler and linker
CC ?= cc
