#!/bin/bash

# Go into the directory containing this script (will be trunk/ of the 
# inferno checkout we wish to work with)
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=`dirname "$SCRIPT"`
cd $SCRIPTPATH

# Set the user as the owner of this script. This is used by sftp in the publish script.
export USER=`stat -c %U $0`

# Name the log file.
LOGFILE=log_`date +%Y%m%d`.txt

# Use a subshell to capture all the stdout, stderr etc
(
    # Logging
    echo "Nightly script run at "`date`" on $HOSTNAME" 
    echo "Running from "`pwd`" USER set to $USER" 
    echo

    # See if any changes have been checked in since we last ran
    # Any argument forces run even if no diffs in subversion
    echo Diffing...
    svn diff -rHEAD | tee diff.txt
    echo -n $1 >> diff.txt

    if [ -s diff.txt ]
    then
        echo Updating...
        # yes, so grab the changes
        svn update

        # build inferno, generate docs and publish them on sourceforge.net
        # Note: for publishing to work non-interactively, you need DSA keys 
        # created and the public key logged with sourceforge. See
        # https://sourceforge.net/apps/trac/sourceforge/wiki/SSH%20keys
        echo Build, generate, publish...
        make publish  

        # Do a test
        make test
    fi
    
    echo Done.
) > $LOGFILE 2>&1 

