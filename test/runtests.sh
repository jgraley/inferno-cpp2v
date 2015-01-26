#!/bin/bash

# runtests.sh
#
# This is the runner for the inferno test suite. It loops over a set of test 
# vectors and invokes whichever test script is required. It attempts to run 
# them in parallel to seepd things up on a multicore machine. See test.sh
# or fulltest.sh for details of the tests performed.
#

# Stop on error (errors in individual tests will be trapped)
set -e

# Decide where things should go
clang_tests=llvm/tools/clang/test
resfile=test/results.csv
logdir=test/results

# Usage:
# runtests.sh - tests the known passing (regression) tests
# runtests.sh all - tests all vectors including some that are expected to fail
# runtests.sh full - direct reference check of each intermediate
# runtests.sh <file list> - tests all named files
if test -z $1
then
    infilelist="test/examples/*"
    testscript="test.sh"
elif test $1 == all
then
    infilelist="test/examples/* $clang_tests/CodeGen/*"
    testscript="test.sh"
elif test $1 == sr
then
    infilelist="test/examples/*"
    testscript="srtest.sh"
elif test $1 == rep
then
    infilelist="test/examples/*"
    testscript="reptest.sh"
else
    infilelist=$*
    testscript="test.sh"
fi

# Clear down output files/directories since we would not like out-of-date results
rm -f $resfile
mkdir -p test/results
rm -rf test/results/*

echo testing files $infilelist

# Prevent the pipe to tee from hiding the status code from test.sh
set -o pipefail

# Start all the tests in seperate processes to take advantage of mult-core host
for infile in $infilelist
do
    # Decide where to put the log file for this test run 
    logfile=$logdir/`basename $infile`.log
    # Start the test and pipe output to tee so we get it on the terminal and in the 
    # log file, but turn off the normal messges because there's too many! 
    test/$testscript $infile -tq 2>&1 | tee $logfile &
done

# Wait for all the tests to complete and count failures 
failed=0
for job in `jobs -p`
do
    # Wait for the numbered job, collect status code and increment the fail count if it was bad
    # TODO could detect rejected tests (status code 2)
    wait $job || let "failed+=1"
done

# Report the overall results after all individual tests have completed
echo
echo ------------------------------------------
echo -n "Tests Run: "
date
echo
if test $failed -eq 0
then
    echo "ALL TESTS PASSED"
else
    echo "$failed TESTS FAILED"
fi

