#!/bin/bash

#
# reptest.sh
#
# This is a test for general repeatability. Inferno has to run the same way
# every time, even in respects that should not affect correctness (eg the ordering
# we get when iterating an unordered container) so that (a) unwanted changes in 
# behaviour don't cloud bug investigations and (b) renders come out consistently 
# so we can compare their output using eg diff, as done in the srtest.
#
# This test runs each test vector all the way through and compares 
#

inferno=./inferno.exe
resfile=test/results.csv

if test $# -eq 0
then
 echo Usage: $0 \<input program\> [\<arguments for inferno\>]
 echo from inferno-cpp2v/trunk/
 exit 1
fi

infile=$1
shift
iargs=$*
fb=`basename $infile`
fbnx=`basename $infile .cpp` # only removes .cpp extension

mkdir -p test/results/
logfile=test/results/$fbnx.trace_log
reflogfile=test/reference/reptest/$fbnx.trace_log
outbase=$outdir/$fbnx

echo
echo -------------- $fb ----------------

# 1000 means "didnt attempt this step due to earlier error"
# ires is inferno result, dres is diff result.
ires=1000
dres=1000
 
return_code=1 
echo Transform... 
# Always do self-test
time $inferno -s -t -i$infile $iargs > $logfile 2>&1 
ires=$?

if test $ires -eq 0
then
 echo Compare output...
 diff $logfile $reflogfile | head -n 100
 dres=$?
 if test $dres -eq 0
 then
  echo $infile PASSED
  return_code=0
 fi
fi

if test -z $resfile
then
 echo $fb "," $ires "," $d
else
 echo $fb "," $ires "," $dres >> $resfile
fi

exit $return_code

