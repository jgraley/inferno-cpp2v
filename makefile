include makefile.common

.PHONY: default all get_libs test docs force_subordinate_makefiles clean iclean resource publish doxygen pattern_graphs doc_graphs
default : inferno.exe
all : clean get_libs inferno.exe resource docs test

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
	patches/apply.sh
								
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
LLVM_CLANG_OPTIONS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED) 
# Strangely, on C++11, we get problems with inferred rvalue refs, and below 11
# the clang code tries to use alignof. We work around the latter here.
LLVM_CLANG_OPTIONS += CXXFLAGS="-include cstdio -include stdint.h -std=c++03 $(ILC_OPTIONS) -fPIC"
LLVM_CLANG_OPTIONS += CFLAGS=$(ILC_OPTIONS)
LLVM_CLANG_OPTIONS += --jobs=$(JOBS)

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
# Link inferno executable
#
STANDARD_LIBS += -lstdc++
BOOST_LIBS += -lboost_context
inferno.exe : makefile makefile.common build/inferno.a $(LLVM_CLANG_LIB_PATHS)
	$(ICC) build/inferno.a $(LLVM_CLANG_LIB_PATHS) $(STANDARD_LIBS) $(BOOST_LIBS) -fuse-ld=gold -ggdb -pg -no-pie -o inferno.exe

#
# Build the doxygen docs
#
doxygen :
	doxygen docs/generated/Doxy-Inferno
	@echo Doxygen documentation now at: $(PWD)/docs/generated/html/index.html

#
# Build the step graphs and stats
#
pattern_graphs : makefile inferno.exe docs/generated/gen_pattern_graphs.sh
	cd docs/generated && ./gen_pattern_graphs.sh

#
# Build the step graphs and stats
#
doc_graphs : makefile inferno.exe docs/generated/gen_doc_graphs.sh
	cd docs/generated && ./gen_doc_graphs.sh

#
# Build all of the generatable documentation
#
docs : doxygen pattern_graphs doc_graphs

#
# Push web site to sourceforge
#
publish : makefile docs inferno.exe docs/web/publish.sh
	cd docs/web && ./publish.sh

#
# Run the main Inferno tests
#
test : test/results/execution.pass
minitest : test/results/mini.pass

#
# Run the search and replace tests (requires reference outputs to be in in test/reference/srtest/)
#
srtest : test/results/sr.pass
	
#
# Run the repeatbility tests (requires reference trace logs to be in in test/reference/reptest/)
#
reptest : test/results/rep.pass
	
#
# Cleaning up
#
clean : makefile $(LLVM_CLANG_LIBS:%=clean_%) iclean

iclean : makefile 
	-rm -rf build/*
	-rm -f inferno.exe
	-rm -f resource/script/* resource/lib/*

#
# Subordinate makefiles included last so they don't hijack the makefile default goal
#
include src/makefile
include test/makefile
include resource/makefile
