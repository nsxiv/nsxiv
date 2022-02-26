[![nsxiv](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/logo.png)](https://github.com/nsxiv/nsxiv)

[![tags](https://img.shields.io/github/v/tag/nsxiv/nsxiv?style=flat-square)](https://github.com/nsxiv/nsxiv/tags)
[![license](https://img.shields.io/badge/license-GPL--2.0-lightgreen?style=flat-square)](https://github.com/nsxiv/nsxiv/blob/master/LICENSE)
[![loc](https://img.shields.io/tokei/lines/github/nsxiv/nsxiv?color=red&style=flat-square)](https://github.com/nsxiv/nsxiv)

> This fork adds the alpha patch, in addition to the dmenu mode and toggle background patches,
> although the last two I'm not sure how are supposed to work/be used.

**Neo (or New or Not) Simple (or Small or Suckless) X Image Viewer**
--------------------------------------------------------------------

nsxiv is a fork of now unmaintained [sxiv](https://github.com/muennich/sxiv)
with the purpose of being a drop-in replacement of sxiv, maintaining it and
adding simple, sensible features. nsxiv is free software licensed under GPLv2
and aims to be easy to modify and customize.

Please file a bug report if something does not work as documented or
expected in *this* repository, after making sure you are using the latest
release of nsxiv. Contributions are welcome, see
[CONTRIBUTING.md](CONTRIBUTING.md#Contribution-Guideline) for details.


Features
--------

* Basic image operations, e.g. zooming, panning, rotating
* Customizable key and mouse button mappings (in *config.h*)
* Script-ability via `key-handler`
* Thumbnail mode: grid of selectable previews of all images
* Ability to cache thumbnails for fast re-loading
* Basic support for animated/multi-frame images (GIF/WebP)
* Display image information in status bar
* Display image name/path in X title


Screenshots
-----------

**Image mode: (Default colors)**

![Image](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/image.png "Image mode")

**Thumbnail mode: (Custom colors)**

![Thumb](https://raw.githubusercontent.com/nsxiv/nsxiv/gh-pages/img/thumb.png "Thumb mode")


Installing via package manager
------------------------------

nsxiv is available on the following distributions/repositories. If you don't see
your distro listed here, either contact your distro's package maintainer or
consider packaging it yourself and adding it to the respective community repo.

[![Packaging status](https://repology.org/badge/vertical-allrepos/nsxiv.svg)](https://repology.org/project/nsxiv/versions)

Repos not tracked by repology:

* Fedora: Enable the copr repo via `dnf copr enable mamg22/nsxiv`.


Dependencies
------------

nsxiv requires the following software to be installed:

  * Imlib2
  * X11

The following dependencies are optional.

  * inotify : Used for auto-reloading images on change.
    Disabled via `HAVE_INOTIFY=0`
  * libXft, freetype2, fontconfig : Used for the status bar.
    Disabled via `HAVE_LIBFONTS=0`
  * giflib : Used for animated gif playback.
    Disabled via `HAVE_LIBGIF=0`.
  * libexif : Used for auto-orientation and exif thumbnails.
    Disable via `HAVE_LIBEXIF=0`
  * libwebp : Used for animated webp playback.
    (NOTE: animated webp also requires Imlib2 v1.7.5 or above)
    Disabled via `HAVE_LIBWEBP=0`.

Please make sure to install the corresponding development packages in case that
you want to build nsxiv on a distribution with separate runtime and development
packages (e.g. \*-dev on Debian).


Building
--------

nsxiv is built using the commands:

    $ make

You can pass `HAVE_X=0` to `make` to disable an optional dependency.
For example:

    $ make HAVE_LIBEXIF=0

will disable `libexif` support. Alternatively they can be disabled via editing
the `Makefile` directly. `OPT_DEP_DEFAULT=0` can be used to disable all
optional dependencies.

Installing nsxiv:

    # make install

Installing desktop entry:

    # make install-desktop

Installing icons:

    # make install-icon

Installing all of the above:

    # make install-all

Please note, that these requires root privileges.
By default, nsxiv is installed using the prefix `/usr/local`, so the full path
of the executable will be `/usr/local/bin/nsxiv`, the `.desktop` entry will be
`/usr/local/share/applications/nsxiv.desktop` and the icon path will be
`/usr/local/share/icons/hicolor/{size}/apps/nsxiv.png`.

You can install nsxiv into a directory of your choice by changing this command to:

    $ make PREFIX="/your/dir" install

Example scripts are installed using `EGPREFIX` which defaults to
`/usr/local/share/doc/nsxiv/examples`. You can change `EGPREFIX` the same way
you can change `PREFIX` shown above.

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
Yes, see [nsxiv-url](https://github.com/nsxiv/nsxiv-extra/tree/master/scripts/nsxiv-url)

* Can I open all the images in a directory? <br>
Yes, see [nsxiv-rifle](https://github.com/nsxiv/nsxiv-extra/tree/master/scripts/nsxiv-rifle)

* Can I set default arguments for nsxiv? <br>
Yes, see [nsxiv-env](https://github.com/nsxiv/nsxiv-extra/tree/master/scripts/nsxiv-env)

* Can I pipe images into nsxiv? <br>
Yes, see [nsxiv-pipe](https://github.com/nsxiv/nsxiv-extra/tree/master/scripts/nsxiv-pipe)

* nsxiv crashes when viewing images with emojis in their name. <br>
This is an issue with libXft. Either wait for
[this](https://gitlab.freedesktop.org/xorg/lib/libxft/-/merge_requests/1) fix to
be merged, or install [libxft-bgra](https://github.com/uditkarode/libxft-bgra)


Customization
-------------

The main method of customizing nsxiv is by setting values for the variables in *config.h*,
or by using Xresources as explained in the manual. If these options are not sufficient,
you may implement your own features by following
[this guide](https://github.com/nsxiv/nsxiv-extra/blob/master/CUSTOMIZATION.md).

Due to our limited [project scope](CONTRIBUTING.md#Project-Scope), certain features or
customization cannot be merged into nsxiv mainline. Following the spirit of suckless
software, we host the [nsxiv-extra](https://github.com/nsxiv/nsxiv-extra) repo where users
are free to submit whatever patches or scripts they wish.

If you think your custom features can be beneficial for the general user base and is within
our project scope, please submit it as a pull request on this repository, then we *may*
merge it to mainline.

Description on how to use or submit patches can be found on
nsxiv-extra's [README](https://github.com/nsxiv/nsxiv-extra).


Download
--------

You can [browse](https://github.com/nsxiv/nsxiv) the source code repository
on GitHub or get a copy using git with the following command:

    $ git clone https://github.com/nsxiv/nsxiv.git

You can view the changelog [here](CHANGELOG.md)

Related projects
----------------

If nsxiv isn't able to fit your needs, check out the image viewer section of
[suckless rocks](https://suckless.org/rocks) to find other minimal image viewers
to try out.

Below are a couple other lesser known projects not listed in suckless rocks.

* [MAGE](https://github.com/explosion-mental/mage) :
  A smaller/more-suckless version of sxiv.
* [div](https://github.com/TAAPArthur/div) :
  Minimal and extensive, aimed at C devs willing to build their own features.
* [mpv-image-viewer](https://github.com/occivink/mpv-image-viewer) :
  Lua script to turn mpv into an image viewer. Supports thumbnails via
  [mpv-gallery-view](https://github.com/occivink/mpv-gallery-view).
