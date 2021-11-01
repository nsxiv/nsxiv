#!/bin/sh

git show "master:nsxiv.1" | groff -mandoc -Thtml |
  sed 's|^</head>|<link rel="stylesheet" href="index.css"></head>|' > index.html
