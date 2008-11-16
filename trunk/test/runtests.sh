#!/bin/bash

clang_tests=../llvm/tools/clang/test
inferno=../inferno.exe
resfile=results.csv
infilelist="examples/* $clang_tests/CodeGen/*"


rm -f $resfile
mkdir -p results
rm -f results/*

for infile in $infilelist
do
 fb=`basename $infile`
 outfile=results/$fb

 echo
 echo -------------- $fb ----------------
 c1res=1000
 ires=1000
 c2res=1000

 echo Compile input...
 # note: we restrict input to ansi since that's the project's objective. But
 # (1) we do not use -pedantic because its infuriating and
 # (2) we do not apply either restriction to intermediate output, which may
 # use gcc extensions etc
 g++ -ansi -c $infile -o results/"$fb"_in.o
 c1res=$?

 if test $c1res -eq 0
 then
  echo Transform... 
  $inferno -i $infile -o $outfile
  ires=$?

  if test $ires -eq 0
  then
   echo Compile output...
   g++ -c $outfile -o results/"$fb"_out.o
   c2res=$?
  fi
 fi
 echo $fb ", " $c1res ", " $ires ", " $c2res >> $resfile
done
