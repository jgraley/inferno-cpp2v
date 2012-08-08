#!/bin/bash

# didn't compile - don't need it
rm ../llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp

# Inferno uses RTTI and wont link with non-RTTI
./kill_no_rtti.sh ../llvm/tools/clang/lib/*/Makefile

# GCC4 puts string lib return values in const char *
patch ../llvm/lib/System/Unix/Signals.inc gcc44_patch1.txt

# GCC with Ubuntu 12.04 requires a #include for ptrdiff_t  
patch ../llvm/include/llvm/ADT/ilist.h ptrdiff_patch1.txt
patch ../llvm/include/llvm/ADT/SmallVector.h ptrdiff_patch2.txt
patch ../llvm/include/llvm/Use.h ptrdiff_patch3.txt

