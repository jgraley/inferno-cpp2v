#!/bin/bash

# Delete lines that include fno-rtti from each of the supoplied files. 
# The idea is to stop makefiles turning off RTTI during compilation

for makefile in $*
do
  grep -v fno-rtti $makefile > temp
  mv temp $makefile
done
