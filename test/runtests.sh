#!/bin/bash


clang_tests=../llvm/tools/clang/test
inferno=../inferno.exe
resfile=results.csv

rm -f $resfile
for infile in $clang_tests/CodeGen/*
do
 fb=`basename $infile`
 outfile=results/$fb

 $inferno -t -i $infile > $outfile
 ires=$?

 gccres=0
 if test $ires -eq 0
 then
  g++ -c $outfile
  gccres=$?
 fi

 echo $fb ", " $ires ", " $gccres >> $resfile
done
