#!/bin/sh

git show "master:nsxiv.1" > nsxiv.1

# mandoc -Thtml nsxiv.1 > index.html
groff -mandoc -Thtml < nsxiv.1 > index.html

rm nsxiv.1
