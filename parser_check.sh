#!/bin/bash -ex

make -j 30

# Render the built-in patterns 
mkdir -p p1
rm -f p1/*
./inferno.exe -p p1/

# Re-render the parsed patterns 
mkdir -p p2
rm -f p2/*
./inferno.exe p1/* -p p2/ -qX

#diff --color p1 p2

TC=small.c
QA=T130

./inferno.exe p1/* -i test/examples/${TC} -q${QA} > out_p1.cpp
./inferno.exe      -i test/examples/${TC} -q${QA} > out_builtin.cpp
diff --color out_p1.cpp out_builtin.cpp

test/exec_test.sh test/examples/${TC} test/resultspb/exec "p1"
