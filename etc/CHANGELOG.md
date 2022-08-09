nsxiv - Changelog
-----------------

**[git](https://codeberg.org/nsxiv/nsxiv.git)**

Changes will only be documented on stable releases. If you're on git/master then
there may be more changes. Please use `git log` to view them.

- - -

***SPECIAL NOTE***: Due to [this incident](https://codeberg.org/nsxiv/nsxiv-tmp/issues/1)
we have moved development over to [CodeBerg](https://codeberg.org/nsxiv/nsxiv).

A lot of the references *below* may now be 404 on GitHub. Any threads which
survived the wipe have been migrated over to [nsxiv-record]. All of the
references *above* can be found on the new main nsxiv repository on CodeBerg.

[nsxiv-record]: https://codeberg.org/nsxiv/nsxiv-record/

- - -

**[v30](https://codeberg.org/nsxiv/nsxiv/archive/v30.tar.gz)**
*(June 15, 2022)*

* Changes:

  * Development and main repository moved over to Codeberg.
    See the special note above for more info.
  * `autoreload_{inotify,nop}.c` merged into a single file, `autoreload.c`. [#263]
  * Moved all configuration related macros to `config.mk`. [#264]
  * `win-title` is now called only when there's change rather than being called
    on each redraw. [#266]

* Added:

  * Added more mimetypes to the `.desktop` entry. [#260]
  * Added `thumb-info` for customizing the statusbar in thumbnail-mode. [#265]
  * Added comments for building on OpenBSD. [#264]

* Fixes:

  * "Too many open file" error due to not closing the win-title script. [#245]
  * `-f` now directly starts in fullscreen mode rather than opening a normal
    window and then going fullscreen. [#251]
  * Broken slideshow on slow systems or fast animations. [#282]
  * Memory leak when removing an image in thumbnail mode. [#247]
  * Correctly setting `_NET_WM_PID`. [#251]
  * Don't override statusbar if info script doesn't exist. [#271]
  * Potential misbehavior regarding font. [#250]

[#245]: https://codeberg.org/nsxiv/nsxiv-record/pulls/245
[#247]: https://codeberg.org/nsxiv/nsxiv-record/pulls/247
[#250]: https://codeberg.org/nsxiv/nsxiv-record/pulls/250
[#251]: https://codeberg.org/nsxiv/nsxiv-record/pulls/251
[#263]: https://codeberg.org/nsxiv/nsxiv-record/pulls/263
[#264]: https://codeberg.org/nsxiv/nsxiv-record/pulls/264
[#260]: https://codeberg.org/nsxiv/nsxiv-record/pulls/260
[#264]: https://codeberg.org/nsxiv/nsxiv-record/pulls/264
[#265]: https://codeberg.org/nsxiv/nsxiv-record/pulls/265
[#266]: https://codeberg.org/nsxiv/nsxiv-record/pulls/266
[#271]: https://codeberg.org/nsxiv/nsxiv-record/pulls/271
[#282]: https://codeberg.org/nsxiv/nsxiv-record/pulls/282

- - -

**[v29](https://github.com/nsxiv/nsxiv/archive/v29.tar.gz)**
*(March 03, 2022)*

* Changes:

  * Window title is now customizeable via `win-title`, cli flag `-T` and related
    config.h options are removed. See `WINDOW TITLE` section of the manpage for
    more info. [#213]
  * Imlib2 cache size is now set based on total memory percentage, by default
    set to 3%. [#184]
  * Removed some non-POSIX extensions in the Makefile. [#225]

* Added:

  * Ability to customize thumbnail mode mouse-bindings via `config.h`. [#167]
  * Option to set statusbar position to top via `config.h`. [#231]
  * New keybinding <kbd>z</kbd> to scroll to center. [#203]

* Fixes:

  * Manpage cleanup: avoid confusing wording and document thumbnail mode
    mouse-bindings. [#186]
  * Wrong jpeg exif orientation with Imlib2 v1.7.5 (and above). [#188]
  * Animation slowdown when zoomed in. [#200]
  * Reset statusbar after failed keyhandler. [#191]
  * Window title not working on certain WMs. [#234]
  * Various compiler warnings. [#197]

[#167]: https://codeberg.org/nsxiv/nsxiv-record/pulls/167
[#184]: https://codeberg.org/nsxiv/nsxiv-record/pulls/184
[#186]: https://codeberg.org/nsxiv/nsxiv-record/pulls/186
[#188]: https://codeberg.org/nsxiv/nsxiv-record/pulls/188
[#191]: https://codeberg.org/nsxiv/nsxiv-record/pulls/191
[#197]: https://codeberg.org/nsxiv/nsxiv-record/pulls/197
[#200]: https://codeberg.org/nsxiv/nsxiv-record/pulls/200
[#203]: https://codeberg.org/nsxiv/nsxiv-record/pulls/203
[#213]: https://codeberg.org/nsxiv/nsxiv-record/pulls/213
[#225]: https://codeberg.org/nsxiv/nsxiv-record/pulls/225
[#231]: https://codeberg.org/nsxiv/nsxiv-record/pulls/231
[#234]: https://codeberg.org/nsxiv/nsxiv-record/pulls/234

- - -

**[v28](https://github.com/nsxiv/nsxiv/archive/v28.tar.gz)**
*(December 12, 2021)*

* Changes:

  * Statusbar made optional via `HAVE_LIBFONTS`. [#95]
  * Remove library auto-detection, use `OPT_DEP_DEFAULT` instead. [#71]
  * Example scripts will now be installed into `EGPREFIX`
    (`$(PREFIX)/share/doc/nsxiv/examples` by default). See README for more
    info. [#86]

* Added:

  * Animated webp support (optional via `HAVE_LIBWEBP`). [#20]
  * New mouse-binding <kbd>Ctrl-Button1</kbd> for relative drag. [#117]
  * Ability to configure colors and fonts in `config.h`. [#115]
  * Ability to configure navigation width area in `config.h`. [#155]
  * Ability to customize the set of modifiers used when processing keybindings
    in `config.h` via `USED_MODMASK`. [#150]
  * Ability to configure Imlib2's cache size for better image (re)loading
    performance in `config.h`. [#171]
  * Cli flag `-0` for sending null-seperated file-list to standard out (`-o`),
    and key-handler and receiving null-seperated file-list via stdin (`-i`).
    [#68] [#141] [#164]
  * Export environment variable `NSXIV_USING_NULL` to key-handler. [#164]
  * Embed new nsxiv icon. [#163]
  * `make install-icon` to install icons. [#80] [#96]
  * `make install-desktop` to install .desktop entry. [#80] [#96]
  * `make install-all` to install everything. [#80] [#96]
  * Configurable `KEYHANDLER_ABORT` in `config.h`. [#91] [#172]
  * Statusbar message upon key-handler activation. [#98]
  * Ability to write custom C functions in `config.h` and use them via
    keybindings. [#76]

* Fixes:

  * Not able to use `KEYHANDLER_ABORT` key (<kbd>Escape</kbd> by default) in
    regular keybindings. [#91]
  * Memory leak related to Xresources. [#134]
  * Memory leak in gif loader. [#165]
  * Better handle gif colormap and prevent out-of-bound access. [#165]
  * Prevent crash when zooming out in very small images. [#178]
  * Removed non-POSIX commands and extensions from `Makefile`. [#71]
  * Regression where nsxiv wouldn't run on non-TrueColor X server. [#114]
  * Wrong comments in `config.h` and description in `manpage`.
    [#105] [#106] [#152]

[#20]: https://codeberg.org/nsxiv/nsxiv-record/pulls/20
[#68]: https://codeberg.org/nsxiv/nsxiv-record/pulls/68
[#71]: https://codeberg.org/nsxiv/nsxiv-record/pulls/71
[#76]: https://codeberg.org/nsxiv/nsxiv-record/pulls/76
[#80]: https://codeberg.org/nsxiv/nsxiv-record/pulls/80
[#86]: https://codeberg.org/nsxiv/nsxiv-record/pulls/86
[#91]: https://codeberg.org/nsxiv/nsxiv-record/pulls/91
[#95]: https://codeberg.org/nsxiv/nsxiv-record/pulls/95
[#96]: https://codeberg.org/nsxiv/nsxiv-record/pulls/96
[#98]: https://codeberg.org/nsxiv/nsxiv-record/pulls/98
[#105]: https://codeberg.org/nsxiv/nsxiv-record/pulls/105
[#106]: https://codeberg.org/nsxiv/nsxiv-record/pulls/106
[#114]: https://codeberg.org/nsxiv/nsxiv-record/pulls/114
[#115]: https://codeberg.org/nsxiv/nsxiv-record/pulls/115
[#117]: https://codeberg.org/nsxiv/nsxiv-record/pulls/117
[#134]: https://codeberg.org/nsxiv/nsxiv-record/pulls/134
[#141]: https://codeberg.org/nsxiv/nsxiv-record/pulls/141
[#150]: https://codeberg.org/nsxiv/nsxiv-record/pulls/150
[#152]: https://codeberg.org/nsxiv/nsxiv-record/pulls/152
[#155]: https://codeberg.org/nsxiv/nsxiv-record/pulls/155
[#163]: https://codeberg.org/nsxiv/nsxiv-record/pulls/163
[#164]: https://codeberg.org/nsxiv/nsxiv-record/pulls/164
[#165]: https://codeberg.org/nsxiv/nsxiv-record/pulls/165
[#171]: https://codeberg.org/nsxiv/nsxiv-record/pulls/171
[#172]: https://codeberg.org/nsxiv/nsxiv-record/pulls/172
[#178]: https://codeberg.org/nsxiv/nsxiv-record/pulls/178

- - -

**[v27.1](https://github.com/nsxiv/nsxiv/archive/v27.1.tar.gz)**
*(September 16, 2021)*

* Fixes:

  * Source tarball failing build [#66]

[#66]: https://codeberg.org/nsxiv/nsxiv-record/pulls/66

- - -

**[v27](https://github.com/nsxiv/nsxiv/archive/v27.tar.gz)**
*(September 16, 2021)*

* Changes:

  * Re-release under the name nsxiv.
  * Xresources `Sxiv.foreground` and `Sxiv.background` changed
    to `Nsxiv.window.foreground` and `Nsxiv.window.background`.
  * Xresources `Sxiv.font` changed to `Nsxiv.bar.font`.
  * Rework the build system. Now by default we'll build with only optional
    dependencies that are already installed. [#19]

* Added:

  * Fill scale mode. [#2]
  * Configurable X window title (via `config.h` and the `-T` flag). [#23]
  * Support custom bar colors via Xresources. [#19]
  * Support custom mark color via Xresources. [#51]
  * Toggle animation playback with <kbd>Ctrl-a</kbd>. [#33]
  * Set `_NET_WM_PID` and `WM_CLIENT_MACHINE` X properties. [#13]
  * Set `ICCCM WM manager` hints. [#12]

* Fixes:

  * Cli flag `-G` not initially setting gamma. [#31]
  * Wrong keybinding description in the manpage. [#14]
  * .desktop entry not advertising webp support. [#15]
  * Prevent crash when embedded into transparent window. [#3]
  * Small memory leak. [#57]
  * Rare crash when showing some GIFs. [#41]
  * Rare event where nsxiv wouldn't close after window being destroyed. [#53]

[#2]: https://codeberg.org/nsxiv/nsxiv-record/pulls/2
[#3]: https://codeberg.org/nsxiv/nsxiv-record/pulls/3
[#12]: https://codeberg.org/nsxiv/nsxiv-record/pulls/12
[#13]: https://codeberg.org/nsxiv/nsxiv-record/pulls/13
[#14]: https://codeberg.org/nsxiv/nsxiv-record/pulls/14
[#15]: https://codeberg.org/nsxiv/nsxiv-record/pulls/15
[#19]: https://codeberg.org/nsxiv/nsxiv-record/pulls/19
[#23]: https://codeberg.org/nsxiv/nsxiv-record/pulls/23
[#31]: https://codeberg.org/nsxiv/nsxiv-record/pulls/31
[#33]: https://codeberg.org/nsxiv/nsxiv-record/pulls/33
[#41]: https://codeberg.org/nsxiv/nsxiv-record/pulls/41
[#51]: https://codeberg.org/nsxiv/nsxiv-record/pulls/51
[#53]: https://codeberg.org/nsxiv/nsxiv-record/pulls/53
[#57]: https://codeberg.org/nsxiv/nsxiv-record/pulls/57


sxiv
----

**Stable releases**

**[v26](https://github.com/nsxiv/nsxiv/archive/v26.tar.gz)**
*(January 16, 2020)*

  * Maintenance release

**[v25](https://github.com/nsxiv/nsxiv/archive/v25.tar.gz)**
*(January 26, 2019)*

  * Support font fallback for missing glyphs
  * Fix busy loop when built without inotify
  * Use background/foreground colors from X resource database

**[v24](https://github.com/nsxiv/nsxiv/archive/v24.tar.gz)**
*(October 27, 2017)*

  * Automatically reload the current image whenever it changes
  * Support embedding into other X windows with -e (e.g. tabbed)
  * New option -p prevents sxiv from creating cache and temporary files
  * Simpler mouse mappings, the most basic features are accessible with the
    mouse only (navigate, zoom, pan)

**[v1.3.2](https://github.com/nsxiv/nsxiv/archive/v1.3.2.tar.gz)**
*(December 20, 2015)*

  * external key handler gets file paths on stdin, not as arguments
  * Cache out-of-view thumbnails in the background
  * Apply gamma correction to thumbnails

**[v1.3.1](https://github.com/nsxiv/nsxiv/archive/v1.3.1.tar.gz)**
*(November 16, 2014)*

  * Fixed build error, caused by delayed config.h creation
  * Fixed segfault when run with -c

**[v1.3](https://github.com/nsxiv/nsxiv/archive/v1.3.tar.gz)**
*(October 24, 2014)*

  * Extract thumbnails from EXIF tags (requires libexif)
  * Zoomable thumbnails, supported sizes defined in config.h
  * Fixed build error with giflib version >= 5.1.0

**[v1.2](https://github.com/nsxiv/nsxiv/archive/v1.2.tar.gz)**
*(April 24, 2014)*

  * Added external key handler, called on keys prefixed with `Ctrl-x`
  * New keybinding `{`/`}` to change gamma (by András Mohari)
  * Support for slideshows, enabled with `-S` option & toggled with `s`
  * Added application icon (created by 0ion9)
  * Checkerboard background for alpha layer
  * Option `-o` only prints files marked with `m` key
  * Fixed rotation/flipping of multi-frame images (gifs)

**[v1.1.1](https://github.com/nsxiv/nsxiv/archive/v1.1.1.tar.gz)**
*(June 2, 2013)*

  * Various bug fixes

**[v1.1](https://github.com/nsxiv/nsxiv/archive/v1.1.tar.gz)**
*(March 30, 2013)*

  * Added status bar on bottom of window with customizable content
  * New keyboard shortcuts `\`/`|`: flip image vertically/horizontally
  * New keyboard shortcut `Ctrl-6`: go to last/alternate image
  * Added own EXIF orientation handling, removed dependency on libexif
  * Fixed various bugs

**[v1.0](https://github.com/nsxiv/nsxiv/archive/v1.0.tar.gz)**
*(October 31, 2011)*

  * Support for multi-frame images & GIF animations
  * POSIX compliant (IEEE Std 1003.1-2001)

**[v0.9](https://github.com/nsxiv/nsxiv/archive/v0.9.tar.gz)**
*(August 17, 2011)*

  * Made key and mouse mappings fully configurable in config.h
  * Complete code refactoring

**[v0.8.2](https://github.com/nsxiv/nsxiv/archive/v0.8.2.tar.gz)**
*(June 29, 2011)*

  * POSIX-compliant Makefile; compiles under NetBSD

**[v0.8.1](https://github.com/nsxiv/nsxiv/archive/v0.8.1.tar.gz)**
*(May 8, 2011)*

  * Fixed fullscreen under window managers, which are not fully EWMH-compliant

**[v0.8](https://github.com/nsxiv/nsxiv/archive/v0.8.tar.gz)**
*(April 18, 2011)*

  * Support for thumbnail caching
  * Ability to run external commands (e.g. jpegtran, convert) on current image

**[v0.7](https://github.com/nsxiv/nsxiv/archive/v0.7.tar.gz)**
*(February 26, 2011)*

  * Sort directory entries when using `-r` command line option
  * Hide cursor in image mode
  * Full functional thumbnail mode, use Return key to switch between image and
    thumbnail mode

**[v0.6](https://github.com/nsxiv/nsxiv/archive/v0.6.tar.gz)**
*(February 16, 2011)*

  * Bug fix: Correctly display filenames with umlauts in window title
  * Basic support of thumbnails

**[v0.5](https://github.com/nsxiv/nsxiv/archive/v0.5.tar.gz)**
*(February 6, 2011)*

  * New command line option: `-r`: open all images in given directories
  * New key shortcuts: `w`: resize image to fit into window; `W`: resize window
    to fit to image

**[v0.4](https://github.com/nsxiv/nsxiv/archive/v0.4.tar.gz)**
*(February 1, 2011)*

  * New command line option: `-F`, `-g`: use fixed window dimensions and apply
    a given window geometry
  * New key shortcut: `r`: reload current image

**[v0.3.1](https://github.com/nsxiv/nsxiv/archive/v0.3.1.tar.gz)**
*(January 30, 2011)*

  * Bug fix: Do not set setuid bit on executable when using `make install`
  * Pan image with mouse while pressing middle mouse button

**[v0.3](https://github.com/nsxiv/nsxiv/archive/v0.3.tar.gz)**
*(January 29, 2011)*

  * New command line options: `-d`, `-f`, `-p`, `-s`, `-v`, `-w`, `-Z`, `-z`
  * More mouse mappings: Go to next/previous image with left/right click,
    scroll image with mouse wheel (horizontally if Shift key is pressed),
    zoom image with mouse wheel if Ctrl key is pressed

**[v0.2](https://github.com/nsxiv/nsxiv/archive/v0.2.tar.gz)**
*(January 23, 2011)*

  * Bug fix: Handle window resizes correctly
  * New keyboard shortcuts: `g`/`G`: go to first/last image; `[`/`]`: go 10
    images back/forward
  * Support for mouse wheel zooming (by Dave Reisner)
  * Added fullscreen mode

**[v0.1](https://github.com/nsxiv/nsxiv/archive/v0.1.tar.gz)**
*(January 21, 2011)*

  * Initial release
