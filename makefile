include makefile.common

.PHONY: default all get_libs test docs force_subordinate_makefiles clean iclean dclean resource publish doxygen pattern_graphs doc_graphs
default : inferno.exe
all : clean get_libs inferno.exe resource docs test

#
# Establish required revisions of external code
#
# previous version 58906, 60777, 61676
LLVM_REVISION ?= 61726

#
# unpack the LLVM snapshot that we use
#
# Note: we used to download LLVM and clang from SourceForge and then
# patch it, but LLVM has moved to github, and while svn access is provided, 
# I seemed to get the wrong version. Now a tarball of the patched LLVM
# snapshot is included in our repo. We link to it, as with systemc.
#
get_libs : makefile
	tar -zxf llvm-${LLVM_REVISION}-patched.tgz
							
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
LLVM_CLANG_ARGS := ENABLE_OPTIMIZED=$(ENABLE_OPTIMIZED) 
# Strangely, on C++11, we get problems with inferred rvalue refs, and below 11
# the clang code tries to use alignof. We work around the latter here.
LLVM_CLANG_ARGS += CXXFLAGS="-include cstdio -include stdint.h -std=c++03 -fPIC"
LLVM_CLANG_ARGS += CFLAGS=$(ILC_OPTIONS)
LLVM_CLANG_ARGS += --jobs=$(JOBS)

$(LLVM_LIB_PATH)/libLLVMBit%.a : force_subordinate_makefiles
	cd llvm/lib/Bitcode/$(patsubst libLLVMBit%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_ARGS)	

clean_libLLVMBit%.a : 
	-cd llvm/lib/Bitcode/$(patsubst clean_libLLVMBit%.a,%,$@) && $(MAKE) $(LLVM_CLANG_ARGS)	clean	

$(LLVM_LIB_PATH)/libLLVM%.a : force_subordinate_makefiles
	cd llvm/lib/$(patsubst libLLVM%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_ARGS)	

clean_libLLVM%.a : 
	-cd llvm/lib/$(patsubst clean_libLLVM%.a,%,$$@) && $(MAKE) $(LLVM_CLANG_ARGS)	clean

$(LLVM_LIB_PATH)/libclang%.a : force_subordinate_makefiles
	cd llvm/tools/clang/lib/$(patsubst libclang%.a,%,$(notdir $@)) && $(MAKE) $(LLVM_CLANG_ARGS)	
    	   	
clean_libclang%.a : 
	-cd llvm/tools/clang/lib/$(patsubst clean_libclang%.a,%,$@) && $(MAKE) $(LLVM_CLANG_ARGS) clean	
    	   	
#
# Link inferno executable
#
STANDARD_LIBS += -lstdc++
BOOST_LIBS += -lboost_context
inferno.exe : makefile makefile.common build/inferno.a $(LLVM_CLANG_LIB_PATHS)
	$(ICC) build/inferno.a $(LLVM_CLANG_LIB_PATHS) $(STANDARD_LIBS) $(BOOST_LIBS) -fuse-ld=gold $(OPTIONS) -no-pie -o inferno.exe

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
# Build the development graphs (patterns and inputs with tracing)
#
dev_graphs : makefile inferno.exe gen_dev_graphs.sh
	./gen_dev_graphs.sh

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
# Cleaning up
#
clean : $(LLVM_CLANG_LIBS:%=clean_%) iclean

iclean :  
	-rm -rf build/*
	-rm -f inferno.exe
	-rm -f resource/lib/*

#
# Subordinate makefiles included last so they don't hijack the makefile default goal
#
include src/makefile
include resource/makefile
include test/makefile
