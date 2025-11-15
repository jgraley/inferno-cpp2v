#!/bin/bash -ex

make -j 30

# Render the built-in patterns 
mkdir -p patterns_builtin
rm -f patterns_builtin/*
./inferno.exe -p patterns_builtin/

# Render the parsed patterns 
mkdir -p patterns_parsed
rm -f patterns_parsed/*
./inferno.exe patterns_builtin/* -p patterns_parsed/ -qX

# Since differences are errors, make the parsed version be red by swapping them on diff command line
diff --color patterns_parsed patterns_builtin
