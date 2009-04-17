LEVEL=..
include ../makefile.common

INFERNO_MODULES = common/refcount common/read_args common/trace 
INFERNO_MODULES += tree/tree tree/type_db tree/itemise_members
INFERNO_MODULES += helpers/misc helpers/search_replace 
INFERNO_MODULES += parse/identifier_tracker 
INFERNO_MODULES += inferno

$(INFERNO_MODULES:%=build/%.o) : build/%.o : %.cpp */*.hpp */*.inc $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb

build/inferno.a : makefile $(INFERNO_MODULES:%=build/%.o)
	mkdir -p build
	$(AR) -r build/inferno.a $(INFERNO_MODULES:%=build/%.o)

