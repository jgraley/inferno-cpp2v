SRC = src
SRC_OPTIONS = $(OPTIONS) -I$(SRC)

COMMON = $(SRC)/common
NODE = $(SRC)/node
HELPERS = $(SRC)/helpers
TREE = $(SRC)/tree
SR = $(SRC)/sr
PARSE = $(SRC)/parse
RENDER = $(SRC)/render
STEPS = $(SRC)/steps
MAIN = $(SRC)/main

COMMON_MODULES = $(COMMON)/standard $(COMMON)/read_args $(COMMON)/trace $(COMMON)/hit_count
NODE_MODULES = $(NODE)/node $(NODE)/itemise 
HELPERS_MODULES = $(HELPERS)/walk $(HELPERS)/simple_compare $(HELPERS)/duplicate
TREE_MODULES = $(TREE)/cpptree $(TREE)/validate $(TREE)/scope $(TREE)/misc $(TREE)/typeof $(TREE)/type_db 
SR_MODULES = $(SR)/systemic_constraint $(SR)/simple_solver
SR_MODULES += $(SR)/query $(SR)/search_replace $(SR)/scr_engine $(SR)/and_rule_engine $(SR)/match_all_agent $(SR)/match_any_agent $(SR)/not_match_agent
SR_MODULES += $(SR)/standard_agent $(SR)/search_container_agent $(SR)/green_grass_agent $(SR)/overlay_agent $(SR)/star_agent $(SR)/slave_agent 
SR_MODULES += $(SR)/transform_of_agent $(SR)/pointer_is_agent $(SR)/conjecture $(SR)/agent 
PARSE_MODULES = $(PARSE)/identifier_tracker $(PARSE)/rc_hold
RENDER_MODULES = $(RENDER)/sort_decls $(RENDER)/graph $(RENDER)/render $(RENDER)/uniquify_identifiers $(RENDER)/doc_graphs
STEPS_MODULES = $(STEPS)/inferno_agents $(STEPS)/split_instance_declarations $(STEPS)/generate_implicit_casts $(STEPS)/generate_stacks $(STEPS)/slave_test $(STEPS)/lower_control_flow $(STEPS)/clean_up $(STEPS)/state_out $(STEPS)/systemc_detection $(STEPS)/to_sc_method $(STEPS)/uncombable $(STEPS)/fall_out
MAIN_MODULES = $(MAIN)/inferno

ALL_MODULES = $(COMMON_MODULES) $(NODE_MODULES) $(HELPERS_MODULES) $(TREE_MODULES) $(SR_MODULES) $(PARSE_MODULES) $(RENDER_MODULES) $(STEPS_MODULES) $(MAIN_MODULES) 

build/inferno.a : $(ALL_MODULES:%=build/%.o)
	mkdir -p build$(NODE)/type_db 
	$(AR) -r build/inferno.a $(ALL_MODULES:%=build/%.o)

$(COMMON_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(NODE_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(HELPERS_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(TREE_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(TREE)/*.hpp $(TREE)/*.inc $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

$(SR_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(SR)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

$(PARSE_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(TREE)/*.hpp $(TREE)/*.inc $(SR)/*.hpp $(PARSE)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

$(RENDER_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(TREE)/*.hpp $(TREE)/*.inc $(SR)/*.hpp $(RENDER)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(STEPS_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(TREE)/*.hpp $(TREE)/*.inc $(SR)/*.hpp $(STEPS)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(MAIN_MODULES:%=build/%.o) : build/%.o : %.cpp $(COMMON)/*.hpp $(NODE)/*.hpp $(HELPERS)/*.hpp $(TREE)/*.hpp $(TREE)/*.inc $(SR)/*.hpp $(PARSE)/*.hpp $(RENDER)/*.hpp $(STEPS)/*.hpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@



 
