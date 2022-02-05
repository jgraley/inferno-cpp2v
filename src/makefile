SRC = src
COMMON = $(SRC)/common
NODE = $(SRC)/node
HELPERS = $(SRC)/helpers
TREE = $(SRC)/tree
SR = $(SRC)/sr
SR_PTRANS = $(SR)/ptrans
SR_CSP = $(SR)/csp
SR_AGENTS = $(SR)/agents
SR_SYM = $(SR)/sym
PARSE = $(SRC)/parse
RENDER = $(SRC)/render
STEPS = $(SRC)/steps
MAIN = $(SRC)/main

SRC_OPTIONS = $(OPTIONS) -I$(SRC)
SRC_SR_OPTIONS = $(SRC_OPTIONS) -I$(SR)

COMMON_MODULES = $(COMMON)/standard $(COMMON)/common $(COMMON)/read_args $(COMMON)/trace $(COMMON)/hit_count $(COMMON)/mismatch $(COMMON)/serial $(COMMON)/progress $(COMMON)/orderable
NODE_MODULES = $(NODE)/containers $(NODE)/node $(NODE)/itemise $(NODE)/match
HELPERS_MODULES = $(HELPERS)/flatten $(HELPERS)/walk $(HELPERS)/simple_compare $(HELPERS)/duplicate $(HELPERS)/transformation
TREE_MODULES = $(TREE)/cpptree $(TREE)/validate $(TREE)/scope $(TREE)/misc $(TREE)/typeof $(TREE)/type_db
SR_MODULES = $(SR)/equivalence $(SR)/cache $(SR)/link $(SR)/query $(SR)/search_replace $(SR)/scr_engine $(SR)/and_rule_engine $(SR)/conjecture $(SR)/subcontainers 
SR_MODULES += $(SR)/the_knowledge $(SR)/vn_transformation 
SR_PTRANS_MODULES = $(SR_PTRANS)/pattern_transformation $(SR_PTRANS)/pattern_transformation_common $(SR_PTRANS)/combine_patterns $(SR_PTRANS)/search_to_compare
SR_AGENTS_MODULES = $(SR_AGENTS)/agent $(SR_AGENTS)/agent_common $(SR_AGENTS)/agent_intermediates $(SR_AGENTS)/standard_agent $(SR_AGENTS)/colocated_agent $(SR_AGENTS)/conjunction_agent $(SR_AGENTS)/disjunction_agent $(SR_AGENTS)/negation_agent
SR_AGENTS_MODULES += $(SR_AGENTS)/search_container_agent $(SR_AGENTS)/green_grass_agent $(SR_AGENTS)/delta_agent $(SR_AGENTS)/star_agent $(SR_AGENTS)/slave_agent 
SR_AGENTS_MODULES += $(SR_AGENTS)/transform_of_agent $(SR_AGENTS)/pointer_is_agent $(SR_AGENTS)/builder_agent
SR_CSP_MODULES = $(SR_CSP)/constraint $(SR_CSP)/symbolic_constraint $(SR_CSP)/solver $(SR_CSP)/simple_solver $(SR_CSP)/solver_holder
SR_SYM_MODULES = $(SR_SYM)/expression $(SR_SYM)/overloads $(SR_SYM)/lambdas $(SR_SYM)/primary_expressions $(SR_SYM)/boolean_operators $(SR_SYM)/comparison_operators $(SR_SYM)/symbol_operators
SR_SYM_MODULES += $(SR_SYM)/rewriters $(SR_SYM)/clutch $(SR_SYM)/sym_solver $(SR_SYM)/truth_table
PARSE_MODULES = $(PARSE)/identifier_tracker $(PARSE)/rc_hold
RENDER_MODULES = $(RENDER)/sort_decls $(RENDER)/graph $(RENDER)/render $(RENDER)/uniquify_identifiers $(RENDER)/doc_graphs
STEPS_MODULES = $(STEPS)/inferno_agents $(STEPS)/split_instance_declarations $(STEPS)/generate_implicit_casts $(STEPS)/generate_stacks $(STEPS)/slave_test $(STEPS)/lower_control_flow 
STEPS_MODULES += $(STEPS)/clean_up $(STEPS)/state_out $(STEPS)/systemc_detection $(STEPS)/to_sc_method $(STEPS)/uncombable $(STEPS)/fall_out
MAIN_MODULES = $(MAIN)/inferno

ALL_MODULES = $(COMMON_MODULES) $(NODE_MODULES) $(HELPERS_MODULES) $(TREE_MODULES) $(SR_MODULES) $(SR_PTRANS_MODULES) $(SR_CSP_MODULES) $(SR_AGENTS_MODULES) $(SR_SYM_MODULES) $(PARSE_MODULES) $(RENDER_MODULES) $(STEPS_MODULES) $(MAIN_MODULES) 

# Include the auto-generated .d files which contain rules for the
# include dependencies.
-include $(ALL_MODULES:%=build/%.d)

build/inferno.a : $(ALL_MODULES:%=build/%.o)
	mkdir -p build$(NODE)/type_db 
	$(AR) -r build/inferno.a $(ALL_MODULES:%=build/%.o)

$(COMMON_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(NODE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(HELPERS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(TREE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

$(SR_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_SR_OPTIONS) $< -o $@

$(SR_PTRANS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_SR_OPTIONS) $< -o $@

$(SR_CSP_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_SR_OPTIONS) $< -o $@

$(SR_AGENTS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_SR_OPTIONS) $< -o $@

$(SR_SYM_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_SR_OPTIONS) $< -o $@

$(PARSE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

$(RENDER_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(STEPS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@ 

$(MAIN_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) -c $(SRC_OPTIONS) $< -o $@

 
