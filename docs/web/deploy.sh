#!/bin/sh
# Script to deploy the HTML in htdocs to sourceforge's web server. 
# Lets you check the site first, and checks in if not done already 
# to ensure version control is up-to-date.

echo Showing website in web browser...
echo Proceed if it all looks OK, or ctrl-C me if not.
firefox htdocs/index.html

#echo Uploading web page...
#sftp $USER@web.sourceforge.net <<EOF
#cd /home/project-web/inferno-cpp2v/htdocs
#put htdocs/*.html
#exit
#EOF

DIFF=`svn diff htdocs`
echo $DIFF
if [ -n "$DIFF" ]
then
    echo Diffs detected, so checking the HTML into SVN...
    svn commit -m "upload to web site" htdocs
fi

echo All done!
