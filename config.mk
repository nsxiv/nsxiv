# nsxiv version
VERSION = 30

# PREFIX for install
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
EGPREFIX = $(PREFIX)/share/doc/nsxiv/examples

# default value for optional dependencies. 1 = enabled, 0 = disabled
OPT_DEP_DEFAULT = 1

# autoreload backend: 1 = inotify, 0 = none
HAVE_INOTIFY = $(OPT_DEP_DEFAULT)

# optional dependencies, see README for more info
HAVE_LIBFONTS = $(OPT_DEP_DEFAULT)
HAVE_LIBGIF   = $(OPT_DEP_DEFAULT)
HAVE_LIBEXIF  = $(OPT_DEP_DEFAULT)
HAVE_LIBWEBP  = $(OPT_DEP_DEFAULT)

# Compiler and linker
CC = c99
# CFLAGS, any optimization flags goes here
CFLAGS = -Wall -pedantic

# icons that will be installed via `make icon`
ICONS = 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png

# Uncomment on OpenBSD
# HAVE_INOTIFY = 0
# lib_fonts_bsd_0 =
# lib_fonts_bsd_1 = -lfreetype
# inc_fonts_bsd_0 =
# inc_fonts_bsd_1 = -I/usr/X11R6/include/freetype2
# LDLIBS = -lz -L/usr/local/lib -L/usr/X11R6/lib $(lib_fonts_bsd_$(HAVE_LIBFONTS))
# CPPFLAGS = -I/usr/local/include -I/usr/X11R6/include $(inc_fonts_bsd_$(HAVE_LIBFONTS))
