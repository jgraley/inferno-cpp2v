include makefile.common
.PHONY: all code/inferno.a get_llvm get_clang llvm clang
all : inferno.exe

#
# Establish required versions of llvm and clang
#
LIBS_VERSION ?= 1

#ifeq( $(LIBS_VERSION), 1 )
    LLVM_REVISION ?= 58906
    CLANG_REVISION ?= 58906
#endif

LLVM_URL ?= http://llvm.org/svn/llvm-project/llvm/trunk
CLANG_URL ?= http://llvm.org/svn/llvm-project/cfe/trunk

#
# Check out llvm and clang 
#
# Note: removing PlistDiagnostics.cpp that we don't need and doesn't seem to compile (version 1)	
get_llvm_clang :
	rm -rf llvm
	svn checkout --revision $(LLVM_REVISION) $(LLVM_URL) llvm
	cd llvm; ./configure
	cd llvm/tools; svn checkout --revision $(CLANG_REVISION) $(CLANG_URL) clang
	rm llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp
				
				
#
# Compile llvm and clang sources
#		
LLVM_CLANG_LIBS =  libclangDriver.a libclangParse.a libclangLex.a libclangBasic.a   
LLVM_CLANG_LIBS += libLLVMBitWriter.a libLLVMBitReader.a libLLVMSupport.a libLLVMSystem.a 	
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)
libLLVMBit%.a :
	cd llvm/lib/Bitcode/$(@:libLLVMBit%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	

libLLVM%.a :
	cd llvm/lib/$(@:libLLVM%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	

libclang%.a :
	cd llvm/tools/clang/lib/$(@:libclang%.a=%); $(MAKE) $(LLVM_CLANG_OPTIONS)	

    	   	
#
# Compile inferno sources
#    	
code/inferno.a :
	cd code; $(MAKE) inferno.a

#
# Link inferno executable
#
STANDARD_LIBS += -lstdc++
inferno.exe : code/inferno.a $(LLVM_CLANG_LIBS)
	$(ICC) code/inferno.a $(CLANG_LIBS) $(LLVM_CLANG_LIBS:%=$(LLVM)/$(BUILD)/lib/%) $(STANDARD_LIBS) -ggdb -o inferno.exe

