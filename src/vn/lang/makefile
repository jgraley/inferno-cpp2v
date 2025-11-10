VNL_DIR=src/vn/lang
REFLEX_DIR = reflex
REFLEX = $(REFLEX_DIR)/bin/reflex
LIBREFLEX = $(REFLEX_DIR)/lib/libreflex.a
REFLEX_INC = $(REFLEX_DIR)/include
REFLEX_OPTIONS = --bison-complete --bison-locations
BISON_OPTIONS = -Wcounterexamples
BISON = bison 

# We need to be more relaxed than we are for most VN code.
VNL_OPTIONS = -O0 -std=c++20 -ggdb3
VNL_OPTIONS += -Ibuild/$(VNL_DIR) -I$(REFLEX_INC) -I. 

VNL_LYPP_MODULES=$(VNL_DIR)/vn_lang

# Native C++ code in here can also include from Inferno/VN
VNL_CPP_OPTIONS = $(VNL_OPTIONS) -I$(SRC) -I$(VN) -I$(VN_LANG) -I$(LLVM)/include -I$(CLANG)/include -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS

# We need to compile the RE/flex headers with different flags, so all sources that include
# them need to be here.
VNL_CPP_MODULES = $(VNL_DIR)/vn_parse $(VN_LANG)/vn_commands $(VN_LANG)/vn_lang_error  

VNL_DEPS = $(VNL_DIR)/makefile

$(VNL_LYPP_MODULES:%=build/%.lpp.hpp) : build/%.lpp.hpp : build/%.lpp.cpp
$(VNL_LYPP_MODULES:%=build/%.lpp.cpp): build/%.lpp.cpp : %.lpp $(VNL_DEPS)
	@mkdir -p $(dir $@)
	$(REFLEX) $(REFLEX_OPTIONS) $< --outfile=$@ --header-file=$(subst .cpp,.hpp,$@)	
	

$(VNL_LYPP_MODULES:%=build/%.ypp.hpp) : build/%.ypp.hpp : build/%.ypp.cpp
$(VNL_LYPP_MODULES:%=build/%.location.hpp) : build/%.location.hpp : build/%.ypp.cpp
$(VNL_LYPP_MODULES:%=build/%.ypp.cpp): build/%.ypp.cpp : %.ypp $(VNL_DEPS) 
	@mkdir -p $(dir $@)
	$(BISON) $(BISON_OPTIONS) -d $< -o $@ 


$(VNL_LYPP_MODULES:%=build/%.lpp.o): build/%.lpp.o : build/%.lpp.cpp build/%.ypp.hpp build/%.lpp.hpp build/%.location.hpp $(VNL_DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(VNL_CPP_OPTIONS) -o $@ 


$(VNL_LYPP_MODULES:%=build/%.ypp.o): build/%.ypp.o : build/%.ypp.cpp build/%.ypp.hpp build/%.lpp.hpp build/%.location.hpp $(VNL_DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(VNL_CPP_OPTIONS) -o $@ 


$(VNL_CPP_MODULES:%=build/%.o) : build/%.o : %.cpp $(VNL_LYPP_MODULES:%=build/%.ypp.hpp) $(VNL_LYPP_MODULES:%=build/%.lpp.hpp) $(VNL_LYPP_MODULES:%=build/%.location.hpp) $(VNL_DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(VNL_CPP_OPTIONS) -o $@
	

build/src/lang.a : $(VNL_LYPP_MODULES:%=build/%.lpp.o) $(VNL_LYPP_MODULES:%=build/%.ypp.o) $(VNL_CPP_MODULES:%=build/%.o) $(VNL_DEPS)
	@mkdir -p build/src 
	$(AR) -r $@ $(VNL_LYPP_MODULES:%=build/%.lpp.o) $(VNL_LYPP_MODULES:%=build/%.ypp.o) $(VNL_CPP_MODULES:%=build/%.o)
