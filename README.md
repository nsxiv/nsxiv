Dependencies
------------

nsxiv requires the following software to be installed:

  * Imlib2
  * X11

The following dependencies are optional.

  * `inotify`<sup>\*</sup>: Used for auto-reloading images on change.
    Disabled via `HAVE_INOTIFY=0`.
  * `libXft`, `freetype2`, `fontconfig`: Used for the status bar.
    Disabled via `HAVE_LIBFONTS=0`.
  * `giflib`: Used for animated gif playback.
    Disabled via `HAVE_LIBGIF=0`.
  * `libexif`: Used for auto-orientation and exif thumbnails.
    Disable via `HAVE_LIBEXIF=0`.
  * `libwebp`: Used for animated webp playback.
    (***NOTE***: animated webp also requires Imlib2 v1.7.5 or above)
    Disabled via `HAVE_LIBWEBP=0`.

Please make sure to install the corresponding development packages in case that
you want to build nsxiv on a distribution with separate runtime and development
packages (e.g. \*-dev on Debian).

\* [inotify][] is a Linux-specific API for monitoring filesystem changes.
  It's not natively available on `*BSD` systems but can be enabed via installing
  and linking against [libinotify-kqueue][].

[inotify]: https://www.man7.org/linux/man-pages/man7/inotify.7.html
[libinotify-kqueue]: https://github.com/libinotify-kqueue/libinotify-kqueue


Building
--------

nsxiv is built using the commands:

    $ make

You can pass `HAVE_X=0` to `make` to disable an optional dependency.
For example:

    $ make HAVE_LIBEXIF=0

will disable `libexif` support. Alternatively they can be disabled via editing
`config.mk`. `OPT_DEP_DEFAULT=0` can be used to disable all optional
dependencies.

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

Refer to the man-page for the documentation:

    $ man nsxiv

You may also view the man-page [online](https://nsxiv.codeberg.page/man/).
However, note that the online man-page might not accurately represent your local
copy.


F.A.Q.
------

* Can I open remote urls with nsxiv? <br>
Yes, see [nsxiv-url](https://codeberg.org/nsxiv/nsxiv-extra/src/branch/master/scripts/nsxiv-url)

* Can I open all the images in a directory? <br>
Yes, see [nsxiv-rifle](https://codeberg.org/nsxiv/nsxiv-extra/src/branch/master/scripts/nsxiv-rifle)

* Can I set default arguments for nsxiv? <br>
Yes, see [nsxiv-env](https://codeberg.org/nsxiv/nsxiv-extra/src/branch/master/scripts/nsxiv-env)

* Can I pipe images into nsxiv? <br>
Yes, see [nsxiv-pipe](https://codeberg.org/nsxiv/nsxiv-extra/src/branch/master/scripts/nsxiv-pipe)

You may also wish to see the [known issues](https://codeberg.org/nsxiv/nsxiv/issues/242).


Customization
-------------

The main method of customizing nsxiv is by setting values for the variables in *config.h*,
or by using Xresources as explained in the manual. If these options are not sufficient,
you may implement your own features by following
[this guide](https://codeberg.org/nsxiv/nsxiv-extra/src/branch/master/CUSTOMIZATION.md).

Due to our limited [project scope](etc/CONTRIBUTING.md#project-scope), certain features or
customization cannot be merged into nsxiv mainline. Following the spirit of suckless
software, we host the [nsxiv-extra](https://codeberg.org/nsxiv/nsxiv-extra) repo where users
are free to submit whatever patches or scripts they wish.

If you think your custom features can be beneficial for the general user base and is within
our project scope, please submit it as a pull request on this repository, then we *may*
merge it to mainline.

Description on how to use or submit patches can be found on
nsxiv-extra's [README](https://codeberg.org/nsxiv/nsxiv-extra).
