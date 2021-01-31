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
refbase=$1
shift
iargs=$*
fb=`basename $infile`
fbnx=`basename $infile .cpp` # only removes .cpp extension

mkdir -p test/results/$fbnx
rm -f test/results/$fbnx/*
outdir=test/results/$fbnx
refdir=$refbase/$fbnx
outbase=$outdir/$fbnx

echo
echo -------------- $fb ----------------

# 1000 means "didnt attempt this step due to earlier error"
# ires is inferno result, dres is diff result.
ires=1000
dres=1000
 
return_code=1 
command="$inferno -s -f -i$infile -o$outbase $iargs"
echo "$command"
# Always do self-test
time $command
ires=$?

dres=0
for file in $(find $outdir -type f | sort); do 
    if [[ $(diff --brief $file ${file/#$outdir/$refdir}) ]]; then  
        printf "\n" 
        echo "${file/#$outdir\//} differs from reference (stopping here):"
        set -x
        diff --color ${file/#$outdir/$refdir} $file
        set +x
        printf "\n" 
        dres=1
        break
    fi
done

if [ $dres -eq 0 ] && [ $ires -eq 0 ]
then
    echo "$infile PASSED"
    return_code=0
fi

if test -z $resfile
then
    echo $fb "," $ires "," $d
else
    echo $fb "," $ires "," $dres >> $resfile
fi

exit $return_code

# To run Beyond Compare on all the results
# bcompare test/reference/srtest/ test/results/

# For 3-way use eg
# bcompare test/reference/srtest/pointeris/pointeris_034.cpp test/results/pointeris/pointeris_034.cpp test/reference/srtest/pointeris/pointeris_033.cpp

# To update the references
# cp -r test/results/*/ test/reference/srtest/
