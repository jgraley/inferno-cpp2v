
# overridable
BUILD ?= Release
CC ?= g++

OUTSIDE_SVN = ../../..
LLVM = $(OUTSIDE_SVN)/llvm-2.3
CLANG = $(LLVM)/tools/clang
INCLUDE_PATH = -I$(LLVM)/include -I$(CLANG)/include
LLVM_OPTIONS = -D__STDC_LIMIT_MACROS
LLVM_OBJECTS = $(LLVM)/lib/*/$(BUILD)/*.o $(LLVM)/lib/*/*/$(BUILD)/*.o
CLANG_OBJECTS = $(CLANG)/lib/*/$(BUILD)/*.o

all : inferno.cpp
	$(CC) $(INCLUDE_PATH) $(LLVM_OPTIONS) inferno.cpp $(LLVM_OBJECTS) $(CLANG_OBJECTS) -lstdc++ -o inferno

.PHONY: all
