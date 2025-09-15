LANG=src/vn/lang
REFLEX_DIR = reflex
REFLEX = $(REFLEX_DIR)/bin/reflex
LIBREFLEX = $(REFLEX_DIR)/lib/libreflex.a
REFLAGS = 
BISON = bison 

CXXFLAGS = -O0 -std=c++20 -ggdb3
CXXFLAGS += -Ibuild/$(LANG) -Ireflex/include -I.

MODULES=$(LANG)/vn_lang

DEPS = $(LANG)/makefile


$(MODULES:%=build/%.lpp.hpp) : build/%.lpp.hpp : build/%.lpp.cpp
$(MODULES:%=build/%.lpp.cpp): build/%.lpp.cpp : %.lpp $(DEPS)
	@mkdir -p $(dir $@)
	$(REFLEX) $(REFLAGS) --bison-complete --bison-locations $< --outfile=$@ --header-file=$(subst .cpp,.hpp,$@)	
	

$(MODULES:%=build/%.ypp.hpp) : build/%.ypp.hpp : build/%.ypp.cpp
$(MODULES:%=build/%.location.hpp) : build/%.location.hpp : build/%.ypp.cpp
$(MODULES:%=build/%.ypp.cpp): build/%.ypp.cpp : %.ypp $(DEPS) 
	@mkdir -p $(dir $@)
	$(BISON) -d $< -o $@


$(MODULES:%=build/%.lpp.o): build/%.lpp.o : build/%.lpp.cpp build/%.lpp.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 


$(MODULES:%=build/%.ypp.o): build/%.ypp.o : build/%.ypp.cpp build/%.ypp.hpp build/%.lpp.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 


build/src/lang.a : $(MODULES:%=build/%.lpp.o) $(MODULES:%=build/%.ypp.o) $(DEPS)
	@mkdir -p build/src 
	$(AR) -r $@ $(MODULES:%=build/%.lpp.o) $(MODULES:%=build/%.ypp.o)
	

$(MODULES:%=build/%_exe): build/%_exe : build/src/lang.a $(DEPS)
	g++ $< $(LIBREFLEX) $(CXXFLAGS) -o $@
	
	
.PHONY: langtest
langtest : build/$(LANG)/vn_lang_exe $(DEPS) 
	./$< $(LANG)/langtest.vn
