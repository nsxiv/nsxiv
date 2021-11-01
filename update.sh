#!/bin/sh

css='<link rel="stylesheet" href="index.css">'
icon_src='https://raw.githubusercontent.com/nsxiv/nsxiv/master/icon/16x16.png'
icon='<link rel="icon" type="image/png" href="'"$icon_src"'" sizes="16x16">'

git show "master:nsxiv.1" | groff -mandoc -Thtml |
  sed 's|^</head>|'"${css}${icon}"'</head>|' > index.html
