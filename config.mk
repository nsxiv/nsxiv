# nsxiv version
VERSION = 29

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

# CFLAGS, any optimization flags goes here
CFLAGS = -std=c99 -Wall -pedantic

# icons that will be installed via `make icon`
ICONS = 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png
