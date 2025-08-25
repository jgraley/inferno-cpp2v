#!/bin/bash -ex

TESTCASE=small.c

make -j 5 

valgrind --leak-check=yes --track-origins=yes --num-callers=25 ./inferno.exe -i test/examples/${TESTCASE} 2> log.txt
