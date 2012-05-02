#!/bin/bash
# Script to deploy the HTML in htdocs to sourceforge's web server. 
# Checks in if not done already to ensure version control is up-to-date.

echo Rebuilding...
cd ../..
make inferno.exe
cd docs/web

echo Generating pattern graphs...
./gen_pattern_graphs.sh

echo Uploading web page...
sftp $USER@web.sourceforge.net <<EOF
cd /home/project-web/inferno-cpp2v/htdocs
put htdocs/*
exit
EOF

DIFF=`svn diff htdocs`
if [ -n "$DIFF" ]
then
    echo Diffs detected, so checking the HTML into SVN...
    svn commit -m "uploaded to sourceforge web server" htdocs
fi

echo All done!
