#!/bin/bash

inferno=../inferno.exe
resfile=results.csv

if test $# -eq 0
then
 echo Usage: $0 \<input program\> [\<results log file\>]
 exit 1
fi

infile=$1
resfile=$2
fb=`basename $infile`
outfile=results/$fb

echo
echo -------------- $fb ----------------

# 1000 means "didnt attempt this step due to earlier error"
# ires is inferno result, cxres is compile, lxres is link, rxres 
# is run and cmpres is equality comparison between run results.
# main() may be omitted in some tests in which case compile
# should pass and link should fail.
ires=1000
c1res=1000
c2res=1000
l1res=1000
l2res=1000
r1res=1000
r2res=1000
cmpres=1000
 
return_code=1 
 
echo Compile input...
# note: we restrict input to ansi since that's the project's objective. But
# (1) we do not use -pedantic because its infuriating and
# (2) we do not apply either restriction to intermediate output, which may
# use gcc extensions etc
g++ -ansi -nostdinc -c $infile -o results/"$fb"_in.o
c1res=$?
if test $c1res -ne 0
then
 # if it doesn't even compile, do nothing
 echo SKIPPED
 exit 0
fi

echo Link input...
g++ results/"$fb"_in.o -o results/"$fb"_in.exe
l1res=$?
if test $l1res -eq 0
then
 echo Run input...
 results/"$fb"_in.exe
 r1res=$?
fi

echo Transform... 
# Always do self-test
time $inferno -s -i$infile -o$outfile
ires=$?

if test $ires -eq 0
then
 echo Compile output...
 g++ -c $outfile -o results/"$fb"_out.o
 c2res=$?
 if test $c2res -eq 0
 then
  if test $l1res -eq 0
  then
   echo Link output...
   g++ results/"$fb"_out.o -o results/"$fb"_out.exe
   l2res=$?
   if test $l2res -eq 0
   then
    echo Run output...
    results/"$fb"_out.exe
    r2res=$?
    cmpres=1
    if test $r1res -eq $r2res
    then
     cmpres=0
     echo PASSED
     return_code=0
    fi
   fi 
  else
   echo PASSED COMPILE ONLY
   return_code=0
  fi
 fi
fi

if test -z $resfile
then
 echo $fb "," $ires "," $c1res "," $c2res "," $l1res "," $l2res "," $r1res "," $r2res "," $cmpres
else
 echo $fb "," $ires "," $c1res "," $c2res "," $l1res "," $l2res "," $r1res "," $r2res "," $cmpres >> $resfile
fi

exit $return_code
