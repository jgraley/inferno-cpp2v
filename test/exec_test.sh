#!/bin/bash -e

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
resfile=test/summary.csv 

if test $# -eq 0
then
    echo "Usage: $0 <input program> <output path> [<arguments for inferno>]"
    echo "Run from inferno-cpp2v/"
    exit 1
fi

input_x_path=$1
shift
outpath=$1
shift
iargs=$*
fb=`basename $input_x_path`
output_x_path=$outpath/$fb

mkdir -p $outpath

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
resource/script/compile.sh --exp-run -c $input_x_path -o $outpath/"$fb"_in.o
c1res=$?
if test $c1res -ne 0
then
 # if it doesn't even compile, do nothing TODO return a failure
 echo TEST VECTOR $input_x_path REJECTED
 exit 2
fi

echo Link input...
resource/script/link.sh -i $outpath/"$fb"_in.o -o $outpath/"$fb"_in.exe
l1res=$?
if test $l1res -eq 0
then
 echo Run input...
 set +e
 LD_LIBRARY_PATH=systemc/lib-linux64/ $outpath/"$fb"_in.exe
 r1res=$?
 set -e
fi

echo Transform... 
# Always do self-test
#gdb -ex run --args $inferno -s -i$input_x_path -o$output_x_path $iargs
time $inferno -i$input_x_path -o$output_x_path $iargs
ires=$?

if test $ires -eq 0
then
 clang-format -i $output_x_path
 echo Compile output...
 resource/script/compile.sh -c $output_x_path -o $outpath/"$fb"_out.o
 c2res=$?
 if test $c2res -eq 0
 then
  if test $l1res -eq 0
  then
   echo Link output...
   resource/script/link.sh -i $outpath/"$fb"_out.o -o $outpath/"$fb"_out.exe
   l2res=$?
   if test $l2res -eq 0
   then
    echo Run output...
    set +e
    LD_LIBRARY_PATH=systemc/lib-linux64/ $outpath/"$fb"_out.exe
    r2res=$?
    set -e
    echo From inferno $r2res
    echo Expected $r1res
    cmpres=1
    if test $r1res -eq $r2res
    then
     cmpres=0
     echo $input_x_path PASSED
     return_code=0
    fi
   fi 
  else
   echo $input_x_path PASSED COMPILE ONLY
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

