#!/bin/bash

#
# srtest.sh
#
# This is a test for the search and replace engine specifically. It runs 
# Inferno on the test input programs and then compares the resulting output
# programs with pre-existing reference output programs. Thus, these must be 
# present. Comparison is done using diff.
#
# This will fail on any change in SR output at any stage since the refs were
# generated even if Inferno is still working correctly. On the other hand, 
# it will not detect faulty step designs.
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

mkdir -p test/results/$fbnx
rm -f test/results/$fbnx/*
outdir=test/results/$fbnx
refdir=test/reference/srtest/$fbnx
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
time $inferno -s -f -i$infile -o$outbase $iargs
ires=$?

if test $ires -eq 0
then
 echo Compare output of each step...
 diff -r $outdir $refdir
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

