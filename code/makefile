
# Build type - Debug or Release. For clang and llvm, ENABLE_OPTIMIZED must be
# 0 for debug and 1 for release
BUILD ?= Debug
ENABLE_OPTIMIZED ?= 0

# Standard tools
CC ?= g++
AR ?= ar
MAKE ?= make

# For boost
# JSG I need this to get the boost includes in a cygwin setup
BOOST_PATH := -I/usr/include/boost-1_33_1 -DBOOST_ENABLE_ASSERT_HANDLER

# Compiling with LLVM and Clang
LLVM := ../llvm
CLANG := $(LLVM)/tools/clang
INCLUDE_PATH := -I$(LLVM)/include -I$(CLANG)/include
LLVM_OPTIONS := -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

# For inferno
INCLUDE_PATH := $(INCLUDE_PATH) -I.
INFERNO_SOURCES := inferno.cpp tree/tree.cpp common/refcount.cpp common/read_args.cpp common/trace.cpp parse/inferno_minimal_action.cpp common/type_info.cpp

all : inferno.cpp
	$(CC) $(INCLUDE_PATH) $(LLVM_OPTIONS) $(INFERNO_SOURCES) $(CLANG_LIBS) $(LLVM_LIBS) $(STANDARD_LIBS) $(BOOST_PATH) -ggdb -o inferno

inferno.a :
	$(CC) -c $(INCLUDE_PATH) $(LLVM_OPTIONS) $(INFERNO_SOURCES) $(BOOST_PATH) -ggdb
	$(AR) -r inferno.a *.o

    
clang :
	cd $(CLANG); $(MAKE) ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)

llvm :
	cd $(LLVM); $(MAKE) ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)
	
.PHONY: all clang llvm
