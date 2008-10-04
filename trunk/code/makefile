
# Build type - Debug or Release. For clang and llvm, ENABLE_OPTIMIZED must be
# 0 for debug and 1 for release
BUILD ?= Debug
ENABLE_OPTIMIZED ?= 0

# Standard tools
CC ?= g++
AR ?= ar
MAKE ?= make

# Some stuff that is not under revision control goes into the same directory
# that contains the SVN root dir (inferno-synth/). This is the path to it.
OUTSIDE_SVN := ../../..

# Including and linking with the LLVM and Clang
LLVM := $(OUTSIDE_SVN)/llvm-2.3
CLANG := $(LLVM)/tools/clang
INCLUDE_PATH := -I$(LLVM)/include -I$(CLANG)/include
LLVM_OPTIONS := -D__STDC_LIMIT_MACROS
LLVM_LIBS := $(LLVM)/$(BUILD)/lib/libLLVM*.a
# JSG added basic at the end since other libs depend on it. Should really specify them all in order TODO
CLANG_LIBS := $(LLVM)/$(BUILD)/lib/libclang*.a $(LLVM)/$(BUILD)/lib/libclangBasic.a 

# For inferno
STANDARD_LIBS := -lstdc++
INCLUDE_PATH := $(INCLUDE_PATH) -I.
INFERNO_SOURCES := inferno.cpp tree/tree.cpp common/refcount.cpp common/read_args.cpp parse/inferno_minimal_action.cpp

all : inferno.cpp
	$(CC) $(INCLUDE_PATH) $(LLVM_OPTIONS) $(INFERNO_SOURCES) $(CLANG_LIBS) $(LLVM_LIBS) $(STANDARD_LIBS) -ggdb -o inferno

clang :
	cd $(CLANG); $(MAKE) ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)

llvm :
	cd $(LLVM); $(MAKE) ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)
	
.PHONY: all clang llvm
