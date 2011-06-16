include makefile.common
.PHONY: default all get_libs test docs force_subordinate_makefiles clean
default : inferno.exe
all : clean get_libs inferno.exe docs test

#
# Establish required revisions of external code
#
# previous version 58906, 60777, 61676
LLVM_REVISION ?= 61726
CLANG_REVISION ?= 61726

LLVM_URL ?= http://llvm.org/svn/llvm-project/llvm/trunk
CLANG_URL ?= http://llvm.org/svn/llvm-project/cfe/trunk

#
# Check out llvm and clang 
#
# Patches:
# - Remove PlistDiagnostics.cpp that we don't need and doesn't seem to compile
# - Remove -no-rtti from clang parser makefile - we do use RTTI and G++ 4.3.3 doesn't 
#   like linking an RTTI subclass of a non-RTTI base class.
get_libs : makefile
	svn checkout --force --revision $(LLVM_REVISION) $(LLVM_URL) llvm
	cd llvm && ./configure
	cd llvm/tools && svn checkout --force --revision $(CLANG_REVISION) $(CLANG_URL) clang
	cd patches && ./apply.sh
								
#
# Compile llvm and clang sources
#		
# Build type - Debug or Release. For clang and llvm, ENABLE_OPTIMIZED must be
# 0 for debug and 1 for release
ENABLE_OPTIMIZED ?= 0
LLVM_BUILD ?= Debug
LLVM_LIB_PATH = $(LLVM)/$(LLVM_BUILD)/lib
LLVM_CLANG_LIBS =  libclangDriver.a libclangParse.a libclangLex.a libclangBasic.a   
LLVM_CLANG_LIBS += libLLVMBitWriter.a libLLVMBitReader.a libLLVMSupport.a libLLVMSystem.a 	
LLVM_CLANG_LIB_PATHS = $(LLVM_CLANG_LIBS:%=$(LLVM_LIB_PATH)/%)
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED) CXXFLAGS="-include cstdio -include stdint.h"

$(LLVM_LIB_PATH)/libLLVMBit%.a : force_subordinate_makefiles
	cd llvm/lib/Bitcode/$(patsubst libLLVMBit%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_OPTIONS)	

clean_libLLVMBit%.a : 
	-cd llvm/lib/Bitcode/$(patsubst clean_libLLVMBit%.a,%,$@) && $(MAKE) $(LLVM_CLANG_OPTIONS)	clean	

$(LLVM_LIB_PATH)/libLLVM%.a : force_subordinate_makefiles
	cd llvm/lib/$(patsubst libLLVM%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_OPTIONS)	

clean_libLLVM%.a : 
	-cd llvm/lib/$(patsubst clean_libLLVM%.a,%,$$@) && $(MAKE) $(LLVM_CLANG_OPTIONS)	clean

$(LLVM_LIB_PATH)/libclang%.a : force_subordinate_makefiles
	cd llvm/tools/clang/lib/$(patsubst libclang%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_OPTIONS)	
    	   	
clean_libclang%.a : 
	-cd llvm/tools/clang/lib/$(patsubst clean_libclang%.a,%,$@) && $(MAKE) $(LLVM_CLANG_OPTIONS) clean	
    	   	
#
# Compile inferno sources
#    	
src/build/inferno.a : force_subordinate_makefiles
	cd src && $(MAKE) --jobs=3 build/inferno.a

#
# Link inferno executable
#
STANDARD_LIBS += -lstdc++
inferno.exe : makefile src/build/inferno.a $(LLVM_CLANG_LIB_PATHS)
	$(ICC) src/build/inferno.a $(LLVM_CLANG_LIB_PATHS) $(STANDARD_LIBS) -ggdb -pg -o inferno.exe

#
# Build the documentation
#
docs : makefile src/*/*.?pp
	doxygen docs/doxygen/Doxy-Inferno
	@echo Documentation now at:
	@echo $(PWD)/docs/doxygen/html/index.html
	
#
# Run the tests
#
test : makefile inferno.exe
	cd test && ./runtests.sh
	
#
# Cleaning up
#
clean : makefile $(LLVM_CLANG_LIBS:%=clean_%)
	-rm -rf src/build/
	-rm -f inferno.exe
	
