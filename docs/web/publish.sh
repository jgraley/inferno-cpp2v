#!/bin/bash
# Script to deploy the HTML in htdocs to sourceforge's web server. 
# Checks in if not done already to ensure version control is up-to-date.

echo Uploading web page...
sftp $USER@web.sourceforge.net <<EOF
cd /home/project-web/inferno-cpp2v/htdocs
mkdir web
cd web
mkdir html
cd html
put html/*
mkdir generated
cd generated
mkdir html
cd html
put ../generated/html/*
exit
EOF

#DIFF=`svn diff htdocs`
#if [ -n "$DIFF" ]
#then
#    echo Diffs detected, so checking the HTML into SVN...
#    svn commit -m "uploaded to sourceforge web server" htdocs
#fi

echo All done!
