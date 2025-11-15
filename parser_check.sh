#!/bin/bash -ex

make -j 30

# Render the built-in patterns 
mkdir -p pb
rm -f pb/*
./inferno.exe -p pb/

# Re-render the parsed patterns 
mkdir -p pp
rm -f pp/*
./inferno.exe pb/* -p pp/ -qX

#diff --color pb pp

TC=small.c
QA=T130

./inferno.exe pb/* -i test/examples/${TC} -q${QA} > p.cpp
./inferno.exe      -i test/examples/${TC} -q${QA} > b.cpp
diff --color b.cpp p.cpp

test/exec_test.sh test/examples/${TC} test/resultspb/exec "pb"
