LANG=src/sr/lang
REFLEX_DIR = reflex
REFLEX = $(REFLEX_DIR)/bin/reflex
LIBREFLEX = $(REFLEX_DIR)/lib/libreflex.a
REFLAGS = 
BISON = bison 

CXXFLAGS = -O2
CXXFLAGS += -Wall -Wunused -Wextra
CXXFLAGS += -I. -Ibuild/$(LANG) -Ireflex/include
# Generate include deps
CXXFLAGS += -MMD

LMODULES=$(LANG)/tokens
YMODULES=$(LANG)/parser
MODULES=$(LMODULES) $(YMODULES)

DEPS = $(LANG)/makefile

# Include the auto-generated .d files which contain rules for the
# include dependencies.
-include $(ALL_MODULES:%=build/%.d)

$(YMODULES:%=build/%.cpp): build/%.cpp : %.ypp $(DEPS) 
	@mkdir -p $(dir $@)
	$(BISON) -d $< -o $@

$(LMODULES:%=build/%.hpp) : build/%.hpp : build/%.cpp
$(LMODULES:%=build/%-location.hpp) : build/%-location.hpp : build/%.cpp

$(LMODULES:%=build/%.cpp): build/%.cpp : %.lpp $(DEPS)
	@mkdir -p $(dir $@)
	$(REFLEX) $(REFLAGS) --bison-complete --bison-locations $< --outfile=$@ --header-file=$(subst .cpp,.hpp,$@)
	
	
$(MODULES:%=build/%.o): build/%.o : build/%.cpp $(DEPS)
	@mkdir -p $(dir $@)
	g++ $< -c $(CXXFLAGS) -o $@ 

langexe: build/$(LANG)/parser.o build/$(LANG)/tokens.o $(DEPS)
	g++ build/$(LANG)/parser.o build/$(LANG)/tokens.o $(LIBREFLEX) $(CXXFLAGS) -o $@
	
.PHONY: langtest

langtest : langexe
	./$< $(LANG)/langtest.vn
