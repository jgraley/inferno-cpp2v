#!/bin/bash -e

if test $# -eq 0
then
    echo "Usage: $0 <input program> <output path>"
    echo "Run from inferno-cpp2v/"
    exit 1
fi

infile=$1
shift
outpath=$1
shift
fb=`basename $infile`
outfile=$outpath/$fb

mkdir -p $outpath

post=

# 1000 means "didnt attempt this step due to earlier error"
# ires is inferno result, cxres is compile, lxres is link, rxres 
# is run and cmpres is equality comparison between run results.
# main() may be omitted in some tests in which case compile
# should pass and link should fail.
cres=1000
lres=1000
rres=1000
 
return_code=500 
 
echo Compile...
resource/script/compile.sh -c $infile -o $outpath/"$fb$post".o
cres=$?
if test $cres -eq 0
then
  echo Link...
  resource/script/link.sh -i $outpath/"$fb$post".o -o $outpath/"$fb$post".exe
  lres=$?
  if test $lres -eq 0
  then
    echo Run...
    LD_LIBRARY_PATH=systemc/lib-linux64/ $outpath/"$fb$post".exe
    rres=$?
    return_code=$rres
  fi
fi

echo $fb "," $cres "," $lres "," $rres 

exit $return_code
