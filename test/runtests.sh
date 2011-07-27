#!/bin/bash

clang_tests=llvm/tools/clang/test
resfile=test/results.csv

# Usage:
# runtests.sh - tests the known passing (regression) tests
# runtests.sh all - tests all vectors including some that are expected to fail
# runtests.sh <file list> - tests all named files
if test -z $1
then
 infilelist="test/examples/*"
else 
 if test $1 == all
 then
  infilelist="test/examples/* $clang_tests/CodeGen/*"
 else
  infilelist=$*
 fi
fi

rm -f $resfile
mkdir -p results
rm -f results/*

echo testing files $infilelist

failed=0
for infile in $infilelist
do
 test/test.sh $infile $resfile
 if test $? -ne 0
 then
  failed=1
 fi
done

echo
echo ------------------------------------------

echo -n "Tests Run: "
date
echo

if test $failed -eq 0
then
 echo "ALL TESTS PASSED"
else
 echo "SOME TESTS FAILED"
fi

