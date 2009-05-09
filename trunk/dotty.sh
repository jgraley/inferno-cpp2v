#!/bin/bash

# Usage: ./dotty.sh <filename>
#
# Where filename is a file in test/examples

./inferno.exe -g -i test/examples/$1 | ../../Graphviz/bin/dotty.exe -
