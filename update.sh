#!/bin/sh

git show "master:nsxiv.1" | groff -mandoc -Thtml |
  sed 's|^</head>|<link rel="stylesheet" href="index.css"><link rel="icon" type="image/png" href="https://raw.githubusercontent.com/GRFreire/nsxiv/master/icon/16x16.png" sizes="16x16"></head>|' > index.html
