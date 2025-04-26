SRC = src
COMMON = $(SRC)/common
NODE = $(SRC)/node
HELPERS = $(SRC)/helpers
TREE = $(SRC)/tree
SR = $(SRC)/sr
SR_AGENTS = $(SR)/agents
SR_CSP = $(SR)/csp
SR_DB = $(SR)/db
SR_PTRANS = $(SR)/ptrans
SR_SYM = $(SR)/sym
SR_UP = $(SR)/up
PARSE = $(SRC)/parse
RENDER = $(SRC)/render
STEPS = $(SRC)/steps
MAIN = $(SRC)/main

SRC_OPTIONS = $(IVN_OPTIONS) -I$(SRC)
SRC_SR_OPTIONS = $(SRC_OPTIONS) -I$(SR)

COMMON_MODULES = $(COMMON)/standard $(COMMON)/common $(COMMON)/read_args $(COMMON)/trace $(COMMON)/hit_count $(COMMON)/mismatch $(COMMON)/serial $(COMMON)/progress $(COMMON)/orderable
COMMON_MODULES += $(COMMON)/lambda_loops
NODE_MODULES = $(NODE)/containers $(NODE)/node $(NODE)/itemise $(NODE)/match $(NODE)/clone $(NODE)/relationship $(NODE)/tree_ptr
HELPERS_MODULES = $(HELPERS)/flatten $(HELPERS)/walk $(HELPERS)/simple_compare $(HELPERS)/simple_duplicate $(HELPERS)/transformation
TREE_MODULES = $(TREE)/cpptree $(TREE)/validate $(TREE)/scope $(TREE)/misc $(TREE)/typeof $(TREE)/type_data
SR_MODULES = $(SR)/link $(SR)/query $(SR)/search_replace $(SR)/scr_engine $(SR)/and_rule_engine $(SR)/conjecture $(SR)/subcontainers 
SR_MODULES += $(SR)/vn_step $(SR)/vn_sequence 
SR_AGENTS_MODULES = $(SR_AGENTS)/agent $(SR_AGENTS)/agent_common $(SR_AGENTS)/agent_intermediates $(SR_AGENTS)/special_agent $(SR_AGENTS)/relocating_agent $(SR_AGENTS)/autolocating_agent 
SR_AGENTS_MODULES += $(SR_AGENTS)/standard_agent $(SR_AGENTS)/conjunction_agent $(SR_AGENTS)/disjunction_agent $(SR_AGENTS)/negation_agent
SR_AGENTS_MODULES += $(SR_AGENTS)/depth_agent $(SR_AGENTS)/green_grass_agent $(SR_AGENTS)/delta_agent $(SR_AGENTS)/star_agent $(SR_AGENTS)/embedded_scr_agent 
SR_AGENTS_MODULES += $(SR_AGENTS)/transform_of_agent $(SR_AGENTS)/pointer_is_agent $(SR_AGENTS)/builder_agent
SR_CSP_MODULES = $(SR_CSP)/constraint $(SR_CSP)/symbolic_constraint 
SR_CSP_MODULES += $(SR_CSP)/solver $(SR_CSP)/value_selector $(SR_CSP)/reference_solver $(SR_CSP)/backjumping_solver $(SR_CSP)/solver_holder $(SR_CSP)/solver_factory $(SR_CSP)/solver_test
SR_DB_MODULES += $(SR_DB)/x_tree_database $(SR_DB)/db_common $(SR_DB)/link_table $(SR_DB)/node_table $(SR_DB)/orderings $(SR_DB)/domain $(SR_DB)/domain_extension $(SR_DB)/lacing $(SR_DB)/db_walk 
SR_DB_MODULES += $(SR_DB)/relation_test $(SR_DB)/cat_relation $(SR_DB)/sc_relation $(SR_DB)/df_relation
SR_DB_MODULES += $(SR_DB)/zone $(SR_DB)/tree_zone $(SR_DB)/free_zone $(SR_DB)/mutator $(SR_DB)/duplicate 
SR_PTRANS_MODULES = $(SR_PTRANS)/pattern_transformation $(SR_PTRANS)/pattern_transformation_common $(SR_PTRANS)/combine_patterns $(SR_PTRANS)/search_to_compare $(SR_PTRANS)/split_disjunctions
SR_SYM_MODULES = $(SR_SYM)/result $(SR_SYM)/expression $(SR_SYM)/lazy_eval $(SR_SYM)/rewriters $(SR_SYM)/clutch $(SR_SYM)/sym_solver 
SR_SYM_MODULES += $(SR_SYM)/boolean_operators $(SR_SYM)/predicate_operators $(SR_SYM)/conditional_operators $(SR_SYM)/symbol_operators $(SR_SYM)/set_operators
SR_SYM_MODULES += $(SR_SYM)/truth_table $(SR_SYM)/truth_table_with_predicates $(SR_SYM)/expression_analysis
SR_UP_MODULES +=  $(SR_UP)/tz_relation $(SR_UP)/patches $(SR_UP)/tree_update
SR_UP_MODULES +=  $(SR_UP)/misc_passes $(SR_UP)/merge_passes $(SR_UP)/ordering_pass $(SR_UP)/overlap_pass $(SR_UP)/inversion_pass $(SR_UP)/complement_pass 
SR_UP_MODULES += $(SR_UP)/gap_finding_pass $(SR_UP)/boundary_pass $(SR_UP)/alt_ordering_checker
PARSE_MODULES = $(PARSE)/identifier_tracker $(PARSE)/rc_hold
RENDER_MODULES = $(RENDER)/sort_decls $(RENDER)/graph $(RENDER)/render $(RENDER)/uniquify_identifiers $(RENDER)/doc_graphs
STEPS_MODULES = $(STEPS)/inferno_agents $(STEPS)/split_instance_declarations $(STEPS)/generate_implicit_casts $(STEPS)/generate_stacks $(STEPS)/lower_control_flow 
STEPS_MODULES += $(STEPS)/clean_up $(STEPS)/state_out $(STEPS)/systemc_detection $(STEPS)/to_sc_method $(STEPS)/uncombable $(STEPS)/fall_out $(STEPS)/test_steps
MAIN_MODULES = $(MAIN)/inferno $(MAIN)/unit_test 

ALL_SR_MODULES = $(SR_MODULES) $(SR_AGENTS_MODULES) $(SR_CSP_MODULES) $(SR_DB_MODULES) $(SR_PTRANS_MODULES) $(SR_SYM_MODULES) $(SR_UP_MODULES) 
ALL_MODULES = $(COMMON_MODULES) $(NODE_MODULES) $(HELPERS_MODULES) $(TREE_MODULES) $(PARSE_MODULES) $(RENDER_MODULES) $(STEPS_MODULES) $(ALL_SR_MODULES) $(MAIN_MODULES)
 
# Include the auto-generated .d files which contain rules for the
# include dependencies.
-include $(ALL_MODULES:%=build/%.d)

build/inferno.a : $(ALL_MODULES:%=build/%.o)
	mkdir -p build$(NODE)/type_db 
	$(AR) -r build/inferno.a $(ALL_MODULES:%=build/%.o)

$(COMMON_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(NODE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(HELPERS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(TREE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

$(SR_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_AGENTS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_CSP_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_DB_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_PTRANS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_SYM_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(SR_UP_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_SR_OPTIONS) -o $@

$(PARSE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

$(RENDER_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(STEPS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(MAIN_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

 
