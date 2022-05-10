# Unstable Branch - Imlib2 Multi-frame Images

This branch uses Imlib2 v1.8.0 (or above) for loading multi-frame images.

Imlib2 currently supports multi-frame gif, webp, png, ico and jxl.

However image loading performance currently is not optimal, image
***reloading*** on the other hand is noticeably faster due to imlib2's cache.

Once the performance issues are resolved we will be making this the default
and depricate `HAVE_LIBGIF` and `HAVE_LIBWEBP` in favor of using Imlib2 for
animated images.

## WARNING

This branch will be rebased regularly, so make sure you're good with git to be
able to deal with that. This branch will also be kept upto date with latest
imlib2 git and may not work on your system imlib2 version.

To package maintainers: DO NOT PACKAGE THIS BRANCH.
This branch is only for users who willingly want to take the risk.
