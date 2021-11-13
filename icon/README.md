img2data
--------

img2data is a helper program to convert images to binary data for embedding in
software. The generated data is used in nsxiv to set the window icons.


How It Works
------------

Each given image is compressed using run-length encoding and a data array is
generated. In these arrays, the four high bits of each byte are the run length
minus one and the lowest four bits are the data, which are indices for the
color array.

See `data.gen.h` or the output of img2data for a better understanding.


Dependencies
------------

img2data requires Imlib2 to be installed.


Building
--------

img2data is built using the command:

    $ CC -Wall -std=c89 -pedantic -lImlib2 img2data.c -o img2data

where `CC` is a C compiler such as `gcc`.


Usage
-----

img2data is used as the following:

    $ ./img2data 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png

You may replace or omit any image, but you must have a `data.gen.h` with at
least 1 image for nsxiv to compile.
