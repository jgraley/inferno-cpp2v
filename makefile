include makefile.common

.PHONY: default all test docs force_subordinate_makefiles clean iclean dclean resource publish doxygen pattern_graphs doc_graphs
default : inferno.exe
all : clean inferno.exe resource docs test
	
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
LLVM_CLANG_ARGS += CXXFLAGS="$(OPTIONS) $(EXE_OPTIONS) $(LC_OPTIONS)"
LLVM_CLANG_ARGS += CFLAGS="$(OPTIONS) $(EXE_OPTIONS) $(LC_OPTIONS)"

# reflex is symlink to RE/flex user-level installation
LIBREFLEX_PATH = reflex/lib/libreflex.a

LIB_PATHS = $(LLVM_CLANG_LIB_PATHS) $(LIBREFLEX_PATH)


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
inferno.exe : makefile makefile.common build/inferno.a build/src/lang.a $(LIB_PATHS)
	$(ICC) build/inferno.a $(LIB_PATHS) $(OPTIONS) $(EXE_OPTIONS) $(LINK_OPTIONS) -o inferno.exe

#
# Build the doxygen docs
#
doxygen :
	doxygen docs/generated/Doxy-Inferno
	@echo Doxygen documentation now at: $(PWD)/docs/generated/html/index.html

#
# Build the step graphs and stats
#
doc_hitcounts : makefile inferno.exe docs/generated/gen_hitcounts.sh
	cd docs/generated && ./gen_hitcounts.sh

#
# Build the pattern graphs and stats HTML (slow)
#
doc_stats_html : makefile inferno.exe docs/generated/gen_doc_graphs.sh
	cd docs/generated && ./gen_doc_graphs.sh

#
# Build the pattern graphs
#
pattern_graphs : makefile inferno.exe gen_graphs.sh
	./gen_graphs.sh --pattern

#
# Build the internmediate graphs (slow)
#
intermediate_graphs : makefile inferno.exe gen_graphs.sh
	./gen_graphs.sh --intermediate

#
# Build all the graphs (slow)
#
all_graphs : pattern_graphs intermediate_graphs

#
# Build all of the generatable documentation
#
docs : doxygen doc_hitcounts doc_graphs

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
