To Do
-----

  * Add support for more embedded thumbnail formats. Right now, nsxiv seems to use
    the smallest one. [Issue #238](https://github.com/muennich/sxiv/issues/238).
  * Load all frames from TIFF files. We have to write our own loader for this to
    happen--just like we did for GIF images--because Imlib2 does not support
    multiple frames. [Issue #241](https://github.com/muennich/sxiv/issues/241).
