#!/bin/bash

#
# test.sh
#
# This runs the standard Inferno test, which runs Inferno on an input C++ 
# or SystemC program, and checks the output by compiling and running both 
# the input and output programs and comparing their result code. Bugs can 
# cause various of these stages to fail, and all are checked. The test is
# only good if the return value truly reflects what the program actually 
# did. Also note that a failure to transform is likely to cause a false 
# pass. TODO add a heuristic check on output .cpp file to ensure it really
# is in the target format, i.e. satisfies invariants, but not if -q is
# specified.
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
outfile=test/results/$fb

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
resource/script/compile.sh $infile test/results/"$fb"_in.o
c1res=$?
if test $c1res -ne 0
then
 # if it doesn't even compile, do nothing TODO return a failure
 echo TEST VECTOR $infile REJECTED
 exit 2
fi

echo Link input...
resource/script/link.sh test/results/"$fb"_in.o test/results/"$fb"_in.exe
l1res=$?
if test $l1res -eq 0
then
 echo Run input...
 test/results/"$fb"_in.exe
 r1res=$?
fi

echo Transform... 
# Always do self-test
time $inferno -s -ap -i$infile -o$outfile $iargs
ires=$?

if test $ires -eq 0
then
 echo Compile output...
 resource/script/compile.sh $outfile test/results/"$fb"_out.o
 c2res=$?
 if test $c2res -eq 0
 then
  if test $l1res -eq 0
  then
   echo Link output...
   resource/script/link.sh test/results/"$fb"_out.o test/results/"$fb"_out.exe
   l2res=$?
   if test $l2res -eq 0
   then
    echo Run output...
    time test/results/"$fb"_out.exe
    r2res=$?
    cmpres=1
    if test $r1res -eq $r2res
    then
     cmpres=0
     echo $infile PASSED
     return_code=0
    fi
   fi 
  else
   echo $infile PASSED COMPILE ONLY
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
