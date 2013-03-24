#!/bin/bash

# Prepare graphs and text files for investigating a transformation bug
# with a particular test case and step. Will produce "before" and "after"
# artifacts so that the step's effect can be seen. Output program, trace 
# and graph are all produced.
#
# Usage: examine_bug.sh <step number> <path to test case>
#
# Run from trunk/ and path should be from trunk/

STEP=$1
BEFORE=$STEP
AFTER=$(($BEFORE + 1))
TESTCASE=$2
DIR=`basename $TESTCASE .cpp`_step_$STEP

mkdir -p $DIR

echo Examining step $STEP with test case $TESTCASE
echo Files in directory $DIR

# Nice textual things
./inferno.exe -i$TESTCASE -q$BEFORE -t 2> /dev/null      | ./iindent.sh > $DIR/before.cpp 
./inferno.exe -i$TESTCASE -q$AFTER  -t 2> $DIR/trace.txt | ./iindent.sh > $DIR/after.cpp  
diff $DIR/before.cpp $DIR/after.cpp > $DIR/diff.cpp
echo Opening text files in Kate
kate $DIR/*.cpp $DIR/*.txt

# Nice pictorial things
./inferno.exe -i$TESTCASE -q$BEFORE -gi 2> /dev/null | dot -Tsvg > $DIR/before.svg 
./inferno.exe -i$TESTCASE -q$AFTER  -gi 2> /dev/null | dot -Tsvg > $DIR/after.svg  
./inferno.exe -gp$STEP | dot -Tsvg > $DIR/step.svg
echo Opening pictures in Gnome Viewer
eog $DIR/step.svg & # begins gnome viewer
