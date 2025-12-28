#!/bin/bash -ex

ARGS=$*

make -j 30

# Render the built-in patterns 
rm -rf p1
mkdir -p p1 
./inferno.exe ${ARGS} -p p1/

# Re-render the parsed patterns 
mkdir -p p2
rm -f p2/*
./inferno.exe ${ARGS} p1 -p p2/ -qX

# All ensuing tests stop after this step
QA=T130+

# Smoke tests: for one test ccase and one quit-after, compare the transformed outputs
TC=small.c
./inferno.exe ${ARGS} p1 -i test/examples/${TC} -q${QA} > out_p1.cpp
./inferno.exe ${ARGS}    -i test/examples/${TC} -q${QA} > out_builtin.cpp
diff --color out_builtin.cpp out_p1.cpp

# Full test run using patterns we previously wrote to p1/
make -j 30 test I="p1 -q${QA}"
