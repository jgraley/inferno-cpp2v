LEVEL=..
include ../makefile.common

COMMON_MODULES = common/standard common/read_args common/trace common/hit_count
NODE_MODULES = node/node node/itemise 
HELPERS_MODULES = helpers/walk  
TREE_MODULES = tree/tree tree/validate tree/scope tree/misc tree/typeof tree/type_db 
SR_MODULES = sr/search_replace sr/soft_patterns sr/coupling sr/conjecture 
PARSE_MODULES = parse/identifier_tracker parse/rc_hold
RENDER_MODULES = render/sort_decls render/graph render/render render/uniquify_identifiers
STEPS_MODULES = steps/split_instance_declarations steps/generate_implicit_casts steps/generate_stacks steps/use_temps_for_params_return steps/for_to_while steps/slave_test steps/lower_control_flow steps/clean_up steps/state_out
MAIN_MODULES = main/inferno

ALL_MODULES = $(COMMON_MODULES) $(NODE_MODULES) $(HELPERS_MODULES) $(TREE_MODULES) $(SR_MODULES) $(PARSE_MODULES) $(RENDER_MODULES) $(STEPS_MODULES) $(MAIN_MODULES) 

build/inferno.a : makefile $(ALL_MODULES:%=build/%.o)
	mkdir -p buildnode/type_db 
	$(AR) -r build/inferno.a $(ALL_MODULES:%=build/%.o)

$(COMMON_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(NODE_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(HELPERS_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(TREE_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp tree/*.hpp tree/*.inc $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(SR_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp sr/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(PARSE_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp tree/*.hpp tree/*.inc sr/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(RENDER_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp tree/*.hpp tree/*.inc sr/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(STEPS_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp tree/*.hpp tree/*.inc sr/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg

$(MAIN_MODULES:%=build/%.o) : build/%.o : %.cpp common/*.hpp node/*.hpp helpers/*.hpp tree/*.hpp tree/*.inc sr/*.hpp parse/*.hpp render/*.hpp steps/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(OPTIONS) $< -o $@ -ggdb -pg



 
