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
get_llvm_clang :
	rm -rf llvm
	svn checkout --revision $(LLVM_REVISION) $(LLVM_URL) llvm
	cd llvm; ./configure
	cd llvm/tools; svn checkout --revision $(CLANG_REVISION) $(CLANG_URL) clang
				
#
# COmpile llvm and clang sources
#		
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED)
# Note: removing PlistDiagnostics.cpp that we don't need and doesn't seem to compile (version 1)	
llvm_clang :
	cd llvm/lib/System; $(MAKE) $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Support; $(MAKE) $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Bitcode/Reader; $(MAKE) $(LLVM_CLANG_OPTIONS)	
	cd llvm/lib/Bitcode/Writer; $(MAKE) $(LLVM_CLANG_OPTIONS)	
	cd llvm/tools/clang/lib/Basic; $(MAKE) $(LLVM_CLANG_OPTIONS)
	cd llvm/tools/clang/lib/Lex; $(MAKE) $(LLVM_CLANG_OPTIONS)
	cd llvm/tools/clang/lib/Parse; $(MAKE) $(LLVM_CLANG_OPTIONS)
	rm llvm/tools/clang/lib/Driver/PlistDiagnostics.cpp
	cd llvm/tools/clang/lib/Driver; $(MAKE) $(LLVM_CLANG_OPTIONS)
    	   	
#
# Compile inferno sources
#    	
code/inferno.a :
	cd code; $(MAKE) inferno.a

#
# Link inferno executable
#
LLVM_LIBS = $(LLVM)/$(BUILD)/lib/libLLVM*.a
# JSG added basic at the end since other libs depend on it. Should really specify them all in order TODO
CLANG_LIBS = $(LLVM)/$(BUILD)/lib/libclang*.a $(LLVM)/$(BUILD)/lib/libclangBasic.a 
STANDARD_LIBS += -lstdc++
inferno.exe : code/inferno.a
	$(ICC) code/inferno.a $(CLANG_LIBS) $(LLVM_LIBS) $(STANDARD_LIBS) -ggdb -o inferno.exe

