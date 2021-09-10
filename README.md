![sxiv](http://muennich.github.com/sxiv/img/logo.png "sxiv")

**Simple X Image Viewer**

The sole purpose of sxiv is to be the perfect image viewer for me. It is free
software so that you can use it and modify it for your needs. Please file a bug
report if something does not work as documented or expected. Contributions are
welcome but there is no guarantee that they will be incorporated.


Features
--------

* Basic image operations, e.g. zooming, panning, rotating
* Customizable key and mouse button mappings (in *config.h*)
* Thumbnail mode: grid of selectable previews of all images
* Ability to cache thumbnails for fast re-loading
* Basic support for multi-frame images
* Load all frames from GIF files and play GIF animations
* Display image information in status bar


Screenshots
-----------

**Image mode:**

![Image](http://muennich.github.com/sxiv/img/image.png "Image mode")

**Thumbnail mode:**

![Thumb](http://muennich.github.com/sxiv/img/thumb.png "Thumb mode")


Dependencies
------------

sxiv requires the following software to be installed:

  * Imlib2
  * X11
  * Xft
  * freetype2
  * fontconfig
  * giflib (optional, automatically enabled if installed)
  * libexif (optional, automatically enabled if installed)

Please make sure to install the corresponding development packages in case that
you want to build sxiv on a distribution with separate runtime and development
packages (e.g. *-dev on Debian).


Building
--------

sxiv is built using the commands:

    $ make
    # make install

Please note, that the latter one requires root privileges.
By default, sxiv is installed using the prefix "/usr/local", so the full path
of the executable will be "/usr/local/bin/sxiv".

Running make will automatically detect if libexif and libgif are available and
enable them if so. CLI arguments will override any automatic detection.
For example:

	$ make HAVE_LIBGIF=0

will always disable libgif.
Alternatively, they can be disabled via editing `config.mk`.

You can install sxiv into a directory of your choice by changing the second
command to:

    # make PREFIX="/your/dir" install

The build-time specific settings of sxiv can be found in the file *config.h*.
Please check and change them, so that they fit your needs.
If the file *config.h* does not already exist, then you have to create it with
the following command:

    $ make config.h


Usage
-----

Please see the [man page](http://muennich.github.com/sxiv/sxiv.1.html) for
information on how to use sxiv.


Download & Changelog
--------------------

You can [browse](https://github.com/muennich/sxiv) the source code repository
on GitHub or get a copy using git with the following command:

    git clone https://github.com/nsxiv/nsxiv.git
