SRC = src
COMMON = $(SRC)/common
NODE = $(SRC)/node
HELPERS = $(SRC)/helpers
TREE = $(SRC)/tree
VN = $(SRC)/vn
VN_AGENTS = $(VN)/agents
VN_CSP = $(VN)/csp
VN_DB = $(VN)/db
VN_PTRANS = $(VN)/ptrans
VN_SYM = $(VN)/sym
VN_UP = $(VN)/up
VN_GRAPH = $(VN)/graph
VN_LANG = $(VN)/lang
CPLUSPLUS = $(SRC)/cplusplus
STEPS = $(SRC)/steps
MAIN = $(SRC)/main

SRC_OPTIONS = $(OPTIONS) $(EXE_OPTIONS) $(IVN_OPTIONS) -I$(SRC)
SRC_VN_OPTIONS = $(SRC_OPTIONS) -I$(VN)

COMMON_MODULES = $(COMMON)/standard $(COMMON)/common $(COMMON)/read_args $(COMMON)/trace $(COMMON)/hit_count $(COMMON)/mismatch $(COMMON)/serial $(COMMON)/progress $(COMMON)/orderable
COMMON_MODULES += $(COMMON)/lambda_loops
NODE_MODULES = $(NODE)/containers $(NODE)/node $(NODE)/itemise $(NODE)/match $(NODE)/clone $(NODE)/relationship $(NODE)/tree_ptr $(NODE)/graphable $(NODE)/syntax 
HELPERS_MODULES = $(HELPERS)/flatten $(HELPERS)/walk $(HELPERS)/simple_compare $(HELPERS)/simple_duplicate $(HELPERS)/transformation
TREE_MODULES = $(TREE)/cpptree $(TREE)/validate $(TREE)/scope $(TREE)/misc $(TREE)/typeof $(TREE)/type_data
VN_MODULES = $(VN)/link $(VN)/query $(VN)/search_replace $(VN)/scr_engine $(VN)/and_rule_engine $(VN)/conjecture $(VN)/subcontainers 
VN_MODULES += $(VN)/vn_step $(VN)/vn_sequence 
VN_AGENTS_MODULES = $(VN_AGENTS)/agent $(VN_AGENTS)/agent_common $(VN_AGENTS)/agent_intermediates $(VN_AGENTS)/special_agent $(VN_AGENTS)/relocating_agent $(VN_AGENTS)/autolocating_agent 
VN_AGENTS_MODULES += $(VN_AGENTS)/standard_agent $(VN_AGENTS)/conjunction_agent $(VN_AGENTS)/disjunction_agent $(VN_AGENTS)/negation_agent
VN_AGENTS_MODULES += $(VN_AGENTS)/depth_agent $(VN_AGENTS)/green_grass_agent $(VN_AGENTS)/delta_agent $(VN_AGENTS)/star_agent $(VN_AGENTS)/embedded_scr_agent 
VN_AGENTS_MODULES += $(VN_AGENTS)/transform_of_agent $(VN_AGENTS)/pointer_is_agent $(VN_AGENTS)/stringize_agent $(VN_AGENTS)/identifier_by_name_agent
VN_AGENTS_MODULES += $(VN_AGENTS)/builder_agent $(VN_AGENTS)/build_identifier_agent $(VN_AGENTS)/build_container_size_agent
VN_CSP_MODULES = $(VN_CSP)/constraint $(VN_CSP)/symbolic_constraint 
VN_CSP_MODULES += $(VN_CSP)/solver $(VN_CSP)/value_selector $(VN_CSP)/reference_solver $(VN_CSP)/backjumping_solver $(VN_CSP)/solver_factory $(VN_CSP)/solver_test
VN_DB_MODULES += $(VN_DB)/x_tree_database $(VN_DB)/db_common $(VN_DB)/link_table $(VN_DB)/node_table $(VN_DB)/orderings $(VN_DB)/domain $(VN_DB)/domain_extension $(VN_DB)/lacing $(VN_DB)/db_walk 
VN_DB_MODULES += $(VN_DB)/relation_test $(VN_DB)/cat_relation $(VN_DB)/sc_relation $(VN_DB)/df_relation
VN_DB_MODULES += $(VN_DB)/zone $(VN_DB)/tree_zone $(VN_DB)/free_zone $(VN_DB)/mutator $(VN_DB)/mutable_zone $(VN_DB)/duplicate 
VN_PTRANS_MODULES = $(VN_PTRANS)/pattern_transformation $(VN_PTRANS)/pattern_transformation_common $(VN_PTRANS)/combine_patterns $(VN_PTRANS)/search_to_compare $(VN_PTRANS)/split_disjunctions
VN_SYM_MODULES = $(VN_SYM)/result $(VN_SYM)/expression $(VN_SYM)/lazy_eval $(VN_SYM)/rewriters $(VN_SYM)/clutch $(VN_SYM)/sym_solver 
VN_SYM_MODULES += $(VN_SYM)/boolean_operators $(VN_SYM)/predicate_operators $(VN_SYM)/conditional_operators $(VN_SYM)/symbol_operators $(VN_SYM)/set_operators
VN_SYM_MODULES += $(VN_SYM)/truth_table $(VN_SYM)/truth_table_with_predicates $(VN_SYM)/expression_analysis
VN_UP_MODULES +=  $(VN_UP)/tz_relation $(VN_UP)/patches $(VN_UP)/scaffold_ops $(VN_UP)/tree_update
VN_UP_MODULES +=  $(VN_UP)/misc_passes $(VN_UP)/merge_passes $(VN_UP)/ordering_pass $(VN_UP)/inversion_pass 
VN_UP_MODULES += $(VN_UP)/gap_finding_pass $(VN_UP)/boundary_pass $(VN_UP)/alt_ordering_checker $(VN_UP)/move_in_pass $(VN_UP)/move_out_pass $(VN_UP)/copy_passes
VN_GRAPH_MODULES = $(VN_GRAPH)/graph $(VN_GRAPH)/doc_graphs
VN_LANG_MODULES = $(VN_LANG)/sort_decls $(VN_LANG)/render $(VN_LANG)/uniquify_identifiers $(VN_LANG)/indenter 
CPLUSPLUS_MODULES = $(CPLUSPLUS)/identifier_tracker $(CPLUSPLUS)/rc_hold $(CPLUSPLUS)/cpprender 
STEPS_MODULES = $(STEPS)/split_instance_declarations $(STEPS)/generate_implicit_casts $(STEPS)/generate_stacks $(STEPS)/lower_control_flow 
STEPS_MODULES += $(STEPS)/clean_up $(STEPS)/state_out $(STEPS)/to_sc_method $(STEPS)/uncombable $(STEPS)/fall_out $(STEPS)/test_steps
STEPS_MODULES += $(STEPS)/systemc_raising $(STEPS)/systemc_from_c_simple $(STEPS)/systemc_lowering
MAIN_MODULES = $(MAIN)/inferno $(MAIN)/unit_test 

ALL_VN_MODULES = $(VN_MODULES) $(VN_AGENTS_MODULES) $(VN_CSP_MODULES) $(VN_DB_MODULES) $(VN_PTRANS_MODULES) $(VN_SYM_MODULES) $(VN_UP_MODULES) $(VN_GRAPH_MODULES) $(VN_LANG_MODULES)
ALL_MODULES = $(COMMON_MODULES) $(NODE_MODULES) $(HELPERS_MODULES) $(TREE_MODULES) $(CPLUSPLUS_MODULES) $(STEPS_MODULES) $(ALL_VN_MODULES) $(MAIN_MODULES)
 
# Include the auto-generated .d files which contain rules for the
# include dependencies.
-include $(ALL_MODULES:%=build/%.d)

build/inferno.a : $(ALL_MODULES:%=build/%.o)
	@mkdir -p build$(NODE)/type_db 
	$(AR) -r build/inferno.a $(ALL_MODULES:%=build/%.o)

$(COMMON_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(NODE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(HELPERS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(TREE_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

$(VN_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_AGENTS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_CSP_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_DB_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_PTRANS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS) 
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_SYM_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_UP_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_GRAPH_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(VN_LANG_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_VN_OPTIONS) -o $@

$(CPLUSPLUS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

$(STEPS_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@ 

$(MAIN_MODULES:%=build/%.o) : build/%.o : %.cpp $(DEPS)
	@mkdir -p $(dir $@)
	$(ICC) $< -c $(SRC_OPTIONS) -o $@

 
include src/vn/lang/makefile
