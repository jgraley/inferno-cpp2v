#!/bin/bash -e

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
resfile=test/summary.csv

if test $# -eq 0
then
    echo "Usage: $0 [-k] <input program> <reference base> [<arguments for inferno>]"
    echo "-k to keep going after a fail"
    echo "Run from inferno-cpp2v/"
    exit 1
fi

keep_going=0
if [ $1 == "-k" ]; then
    keep_going=1
    shift
fi
infile=$1
shift
refbase=$1
shift
iargs=$*
fb=`basename $infile`
fbnx=`basename $infile .cpp` # only removes .cpp extension

rm -f test/results/$fbnx/*
outdir=test/results/sr/$fbnx
refdir=$refbase/$fbnx
outbase=$outdir/$fbnx

mkdir -p $outdir
rm -rf $outdir/*

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

./convert_all_dot.sh -d $outdir

dres=0
for file in $(find $outdir -type f -name '*.cpp' | sort); do 
    if [[ $(diff --brief $file ${file/#$outdir/$refdir}) ]]; then  
        printf "\n" 
        echo "${file/#$outdir\//} differs from reference (stopping here):"
        set -x
        diff --color ${file/#$outdir/$refdir} $file || :
        set +x
        printf "\n" 
        dres=1
        if [ $keep_going -eq 0 ]; then
            break
        fi
    fi
done

if [ $dres -eq 0 ] && [ $ires -eq 0 ]; then
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
# bcompare test/reference/sr/ test/results/sr/

# For 3-way use eg
# bcompare test/reference/sr/pointeris/pointeris_034.cpp test/results/sr/pointeris/pointeris_034.cpp test/reference/sr/pointeris/pointeris_033.cpp

# To update the references
# cp -r test/results/sr test/reference/
