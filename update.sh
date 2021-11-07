#!/bin/sh

# Dependencies: cat, lowdown, groff

#Mainpage
cat template/template-1.html > index.html
git show "master:README.md" | lowdown | sed -f template/changes.sed >> index.html
cat template/template-2.html >> index.html

#Manpage
css='<link rel="stylesheet" href="../style.css">'
icon_src='https://raw.githubusercontent.com/nsxiv/nsxiv/master/icon/16x16.png'
icon='<link rel="icon" type="image/png" href="'"$icon_src"'" sizes="16x16">'

git show "master:nsxiv.1" | groff -mandoc -Thtml | sed 's|^</head>|'"${css}${icon}"'</head>|' > man/index.html
