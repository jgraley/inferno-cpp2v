# Standard tools
CC ?= g++
AR ?= ar
MAKE ?= make


ENABLE_OPTIMIZED ?= 0

LIBS_REVISION ?= 1

#ifeq( $(LIBS_REVISION), 1 )
    LLVM_REVISION ?= 58906
    CLANG_REVISION ?= 58906
#endif

LLVM_URL ?= http://llvm.org/svn/llvm-project/llvm/trunk
CLANG_URL ?= http://llvm.org/svn/llvm-project/cfe/trunk

get_llvm :
	svn checkout --revision $(LLVM_REVISION) $(LLVM_URL) llvm
	cd llvm; ./configure

get_clang :
	cd llvm/tools; svn checkout --revision $(CLANG_REVISION) $(CLANG_URL) clang
	
		
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)
llvm :
	cd llvm/lib/System; make $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Support; make $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Bitcode/Reader; make $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Bitcode/Writer; make $(LLVM_CLANG_OPTIONS)	
	
clang :	
	cd llvm/tools/clang/lib/Basic; make $(LLVM_CLANG_OPTIONS)
	cd llvm/tools/clang/lib/Lex; make $(LLVM_CLANG_OPTIONS)
	cd llvm/tools/clang/lib/Parse; make $(LLVM_CLANG_OPTIONS)
	rm llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp
	cd llvm/tools/clang/lib/Driver; make $(LLVM_CLANG_OPTIONS)
    	
# Including and linking with the LLVM and Clang
BUILD := Debug
LLVM := llvm
CLANG := $(LLVM)/tools/clang
INCLUDE_PATH := -I$(LLVM)/include -I$(CLANG)/include
LLVM_OPTIONS := -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
LLVM_LIBS := $(LLVM)/$(BUILD)/lib/libLLVM*.a
# JSG added basic at the end since other libs depend on it. Should really specify them all in order TODO
CLANG_LIBS := $(LLVM)/$(BUILD)/lib/libclang*.a $(LLVM)/$(BUILD)/lib/libclangBasic.a 
STANDARD_LIBS := -lstdc++

all : code/inferno.a
	$(CC) code/inferno.a $(CLANG_LIBS) $(LLVM_LIBS) $(STANDARD_LIBS) -ggdb -o inferno

.PHONY: all get_llvm get_clang llvm clang
