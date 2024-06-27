# Migrating from `sxiv`

`nsxiv` is *mostly* a drop-in replacement for `sxiv`, but not fully.
This document outlines some key differences to be aware of if you're migrating
from `sxiv`.

### Configuration directory

`sxiv` looks for config files under the directory
`${XDG_CONFIG_HOME:-${HOME}/.config}/sxiv`. E.g
`~/.config/sxiv/exec/key-handler`.

`nsxiv` uses the same logic to find the config dir but uses the name "nsxiv".
E.g `~/.config/nsxiv/...`.

The "exec" scripts such as `key-handler` and `image-info` in `nsxiv` has some
more features, but all previous argument order are preserved. And so if you have
any exec scripts, you can simply copy them over and they should just work.

### Xresources

The xresources config for `nsxiv` is under the "Nsxiv" namespace whereas `sxiv`
uses the "Sxiv" namespace. Some of the variables are also different between
`nsxiv` and `sxiv`, below is a table that shows the old and new names:

| sxiv                | nsxiv                    |
| :--                 | :--                      |
| Sxiv.background     | Nsxiv.window.background  |
| Sxiv.foreground     | Nsxiv.window.foreground  |
| Sxiv.barBackground  | Nsxiv.bar.background     |
| Sxiv.barForeground  | Nsxiv.bar.foreground     |
| Sxiv.font           | Nsxiv.bar.font           |

### Default window class

The window class of `nsxiv` is set to "Nsxiv" by default (can be overwritten via
`-N` flag). This usually shouldn't matter, unless you have scripts that search
for "Sxiv" window class.

### Thumbnail cache directory

Similar to config dir, the thumbnail cache dir of `nsxiv` is under the "nsxiv"
name instead of "sxiv". E.g `~/.cache/nsxiv`.

The "caching structure" in `nsxiv` is the same as `sxiv`. Which means that you
can simply rename the directory to `nsxiv`:

```console
$ mv  ~/.cache/sxiv  ~/.cache/nsxiv
```

If you want to have both `sxiv` and `nsxiv` installed at the same time, you can
even use symlink to avoid duplicate cache.
