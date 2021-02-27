#!/bin/bash

# didn't compile - don't need it
rm -f llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp

# Inferno uses RTTI and wont link with non-RTTI
patches/kill_no_rtti.sh llvm/tools/clang/lib/*/Makefile

# GCC4.4 puts string lib return values in const char *
patch llvm/lib/System/Unix/Signals.inc patches/gcc44_patch1.txt

# GCC with Ubuntu 12.04 requires a #include for ptrdiff_t  
patch llvm/include/llvm/ADT/ilist.h patches/ptrdiff_patch1.txt
patch llvm/include/llvm/ADT/SmallVector.h patches/ptrdiff_patch2.txt
patch llvm/include/llvm/Use.h patches/ptrdiff_patch3.txt

# GCC4.7 fussy about something or other
patch llvm/include/llvm/Support/CommandLine.h patches/gcc47_patch1.txt

# C++11 fix: LLVM defined it's own alignof in a silly way.
patch llvm/include/llvm/Support/Alignof.h patches/alignof_patch1.txt
patch llvm/include/llvm/ADT/StringMap.h patches/alignof_patch2.txt


