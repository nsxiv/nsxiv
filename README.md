![nsxiv](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/logo.png "nsxiv")

**Neo (or New or Not) Simple (or Small or Suckless) X Image Viewer**
--------------------------------------------------------------------

nsxiv is a fork of now unmaintained [sxiv](https://github.com/muennich/sxiv)
with the purpose of maintaining it and adding simple, sensible features.
nsxiv is free software licensed under GPLv2 and aims to be easy to modify and customize.

Please file a bug report if something does not work as documented or
expected in *this* repository, after making sure you are using the latest
release of nsxiv. Contributions are welcome, see [CONTRIBUTING.md](CONTRIBUTING.md)
for details.


Features
--------

* Basic image operations, e.g. zooming, panning, rotating
* Customizable key and mouse button mappings (in [*config.h*](config.h))
* Thumbnail mode: grid of selectable previews of all images
* Ability to cache thumbnails for fast re-loading
* Basic support for multi-frame images
* Play GIF animations
* Display image information in status bar
* Display image name/path in X title


Screenshots
-----------

**Image mode:**

![Image](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/image.png "Image mode")

**Thumbnail mode and different configurations:**

![Thumb](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/thumb.png "Thumb mode")


Dependencies
------------

nsxiv requires the following software to be installed:

  * Imlib2
  * X11
  * Xft
  * freetype2
  * fontconfig
  * giflib (optional, automatically enabled if installed)
  * libexif (optional, automatically enabled if installed)

Please make sure to install the corresponding development packages in case that
you want to build nsxiv on a distribution with separate runtime and development
packages (e.g. \*-dev on Debian).


Building
--------

nsxiv is built using the commands:

    $ make

Running make will automatically detect if libexif and libgif are available and
enable them if so. CLI arguments will override any automatic detection.
For example:

    $ make HAVE_LIBGIF=0

will always disable libgif.
Alternatively, they can be disabled via editing `config.mk`.

Installing nsxiv:

    # make install

Please note, that this requires root privileges.
By default, nsxiv is installed using the prefix "/usr/local", so the full path
of the executable will be "/usr/local/bin/nsxiv".

You can install nsxiv into a directory of your choice by changing this command to:

    $ make PREFIX="/your/dir" install

The build-time specific settings of nsxiv can be found in the file *config.h*.
Please check and change them, so that they fit your needs.
If the file *config.h* does not already exist, then you have to create it with
the following command:

    $ make config.h


Usage
-----

Please see man page for information on how to use nsxiv. To do so, execute the
following after the installation:

    $ man nsxiv


F.A.Q
-----

* Can I open remote urls with nsxiv? <br>
Yes, see [nsxiv-url](https://github.com/nsxiv/nsxiv/wiki/nsxiv-url)

* Can I open all the images in a directory? <br>
Yes, see [nsxiv-rifle](https://github.com/nsxiv/nsxiv/wiki/nsxiv-rifle)

* Can I set default arguments for nsxiv? <br>
Yes, see [nsxiv-env](https://github.com/nsxiv/nsxiv/wiki/nsxiv-env)

* Can I pipe images into nsxiv? <br>
No, not yet. See https://github.com/nsxiv/nsxiv/issues/32


Download
--------

You can [browse](https://github.com/nsxiv/nsxiv) the source code repository
on GitHub or get a copy using git with the following command:

    $ git clone https://github.com/nsxiv/nsxiv.git
