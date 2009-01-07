include makefile.common
.PHONY: all code/build/inferno.a get_libs test
all : inferno.exe

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
#  Remove PlistDiagnostics.cpp that we don't need and doesn't seem to compile
get_libs : makefile
	rm -rf llvm
	svn checkout --revision $(LLVM_REVISION) $(LLVM_URL) llvm
	cd llvm; ./configure
	cd llvm/tools; svn checkout --revision $(CLANG_REVISION) $(CLANG_URL) clang
	rm llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp
								
#
# Compile llvm and clang sources
#		
# Build type - Debug or Release. For clang and llvm, ENABLE_OPTIMIZED must be
# 0 for debug and 1 for release
ENABLE_OPTIMIZED ?= 0
LLVM_BUILD ?= Debug
LLVM_CLANG_LIBS =  libclangDriver.a libclangParse.a libclangLex.a libclangBasic.a   
LLVM_CLANG_LIBS += libLLVMBitWriter.a libLLVMBitReader.a libLLVMSupport.a libLLVMSystem.a 	
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)

libLLVMBit%.a : makefile
	cd llvm/lib/Bitcode/$(@:libLLVMBit%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	

libLLVM%.a : makefile
	cd llvm/lib/$(@:libLLVM%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	

libclang%.a : makefile
	cd llvm/tools/clang/lib/$(@:libclang%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	
    	   	
#
# Compile inferno sources
#    	
code/build/inferno.a : makefile
	cd code; $(MAKE) build/inferno.a

#
# Link inferno executable
#
STANDARD_LIBS += -lstdc++
inferno.exe : makefile code/build/inferno.a $(LLVM_CLANG_LIBS)
	$(ICC) code/build/inferno.a $(LLVM_CLANG_LIBS:%=$(LLVM)/$(LLVM_BUILD)/lib/%) $(STANDARD_LIBS) -ggdb -o inferno.exe

#
# Run the tests
#
test : makefile inferno.exe
	cd test; ./runtests.sh