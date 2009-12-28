LEVEL=..
include ../makefile.common

INFERNO_MODULES = common/common common/refcount common/read_args common/trace 
INFERNO_MODULES += tree/tree tree/generics tree/type_db tree/itemise
INFERNO_MODULES += helpers/scope helpers/misc helpers/typeof helpers/walk helpers/search_replace helpers/soft_patterns
INFERNO_MODULES += parse/identifier_tracker 
INFERNO_MODULES += render/sort_decls 
INFERNO_MODULES += steps/split_instance_declarations steps/generate_implicit_casts steps/generate_stacks steps/use_temps_for_params_return steps/for_to_while
INFERNO_MODULES += inferno

$(INFERNO_MODULES:%=build/%.o) : build/%.o : %.cpp */*.hpp */*.inc $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb

build/inferno.a : makefile $(INFERNO_MODULES:%=build/%.o)
	mkdir -p build
	$(AR) -r build/inferno.a $(INFERNO_MODULES:%=build/%.o)

