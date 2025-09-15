LANG=src/vn/lang
REFLEX_DIR = reflex
REFLEX = $(REFLEX_DIR)/bin/reflex
LIBREFLEX = $(REFLEX_DIR)/lib/libreflex.a
REFLAGS = 
BISON = bison 

CXXFLAGS = -O0 -std=c++20 -ggdb3
CXXFLAGS += -Ibuild/$(LANG) -Ireflex/include -I.

LYPP_MODULES=$(LANG)/vn_lang

DEPS = $(LANG)/makefile


$(LYPP_MODULES:%=build/%.lpp.hpp) : build/%.lpp.hpp : build/%.lpp.cpp
$(LYPP_MODULES:%=build/%.lpp.cpp): build/%.lpp.cpp : %.lpp $(DEPS)
	@mkdir -p $(dir $@)
	$(REFLEX) $(REFLAGS) --bison-complete --bison-locations $< --outfile=$@ --header-file=$(subst .cpp,.hpp,$@)	
	

$(LYPP_MODULES:%=build/%.ypp.hpp) : build/%.ypp.hpp : build/%.ypp.cpp
$(LYPP_MODULES:%=build/%.location.hpp) : build/%.location.hpp : build/%.ypp.cpp
$(LYPP_MODULES:%=build/%.ypp.cpp): build/%.ypp.cpp : %.ypp $(DEPS) 
	@mkdir -p $(dir $@)
	$(BISON) -d $< -o $@


$(LYPP_MODULES:%=build/%.lpp.o): build/%.lpp.o : build/%.lpp.cpp build/%.lpp.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 


$(LYPP_MODULES:%=build/%.ypp.o): build/%.ypp.o : build/%.ypp.cpp build/%.ypp.hpp build/%.lpp.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 


build/src/lang.a : $(LYPP_MODULES:%=build/%.lpp.o) $(LYPP_MODULES:%=build/%.ypp.o) $(DEPS)
	@mkdir -p build/src 
	$(AR) -r $@ $(LYPP_MODULES:%=build/%.lpp.o) $(LYPP_MODULES:%=build/%.ypp.o)
	

$(LYPP_MODULES:%=build/%_exe): build/%_exe : build/src/lang.a $(DEPS)
	g++ $< $(LIBREFLEX) $(CXXFLAGS) -o $@
	
	
.PHONY: langtest
langtest : build/$(LANG)/vn_lang_exe $(DEPS) 
	./$< $(LANG)/langtest.vn
