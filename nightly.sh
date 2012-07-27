#!/bin/bash

# Go into the directory containing this script (will be trunk/ of the 
# inferno checkout we wish to work with)
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=`dirname "$SCRIPT"`
cd $SCRIPTPATH
LOGFILE=log_`date +%Y%m%d`.txt
export USER=jgraley

# Use a subshell to capture all the stdout, stderr etc
(
    # Logging
    echo "Nightly script run at "`date`" on $HOSTNAME" 

    # See if any changes have been checked in since we last ran
    svn diff -rHEAD > diff.txt

    if [ -s diff.txt ]
    then
        # yes, so grab the changes
        svn update

        # build inferno, generate docs and publish them on sourceforge.net
        # Note: for publishing to work non-interactively, you need DSA keys 
        # created and the public key logged with sourceforge. See
        # https://sourceforge.net/apps/trac/sourceforge/wiki/SSH%20keys
        make publish  
    fi
) > $LOGFILE 2>&1 
