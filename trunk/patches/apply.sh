#!/bin/bash

# didn't compile - don't need it
rm ../llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp

# Inferno uses RTTI and wont link with non-RTTI
./kill_no_rtti.sh ../llvm/tools/clang/lib/*/Makefile

# GCC4 puts string lib return values in const char *
patch ../llvm/lib/System/Unix/Signals.inc gcc44_patch1.txt

