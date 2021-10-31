#!/bin/sh

git show "master:nsxiv.1" | groff -mandoc -Thtml > index.html
awk '/<style type="text\/css">/,/<\/style>/ { if ( $0 ~ /<\/style>/ ) print "<link rel=\"stylesheet\" href=\"index.css\">"; next } 1' \
    index.html > index.html.tmp && mv index.html.tmp index.html
