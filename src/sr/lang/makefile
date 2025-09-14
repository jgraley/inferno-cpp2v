LANG=src/sr/lang
REFLEX_DIR = reflex
REFLEX = $(REFLEX_DIR)/bin/reflex
LIBREFLEX = $(REFLEX_DIR)/lib/libreflex.a
REFLAGS = 
BISON = bison 

CXXFLAGS = -O0 -std=c++20 -ggdb3
CXXFLAGS += -Ibuild/$(LANG) -Ireflex/include -I.

MODULES=$(LANG)/vn_lang

DEPS = $(LANG)/makefile


$(MODULES:%=build/%.ll.hpp) : build/%.ll.hpp : build/%.ll.cpp
$(MODULES:%=build/%.ll.cpp): build/%.ll.cpp : %.lpp $(DEPS)
	@mkdir -p $(dir $@)
	$(REFLEX) $(REFLAGS) --bison-complete --bison-locations $< --outfile=$@ --header-file=$(subst .cpp,.hpp,$@)	
	

$(MODULES:%=build/%.yy.hpp) : build/%.yy.hpp : build/%.yy.cpp
$(MODULES:%=build/%.location.hpp) : build/%.location.hpp : build/%.yy.cpp
$(MODULES:%=build/%.yy.cpp): build/%.yy.cpp : %.ypp $(DEPS) 
	@mkdir -p $(dir $@)
	$(BISON) -d $< -o $@


$(MODULES:%=build/%.ll.o): build/%.ll.o : build/%.ll.cpp build/%.ll.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 

$(MODULES:%=build/%.yy.o): build/%.yy.o : build/%.yy.cpp build/%.yy.hpp build/%.ll.hpp build/%.location.hpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 


$(MODULES:%=build/%_exe): build/%_exe : build/%.ll.o build/%.yy.o
	g++ $^ $(LIBREFLEX) $(CXXFLAGS) -o $@
	
	
.PHONY: langtest
langtest : build/$(LANG)/vn_lang_exe
	./$< $(LANG)/langtest.vn
