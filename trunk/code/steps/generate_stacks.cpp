/*
 * generate_stacks.cpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#include "steps/generate_stacks.hpp"
#include "tree/tree.hpp"
#include "common/common.hpp"
#include "common/refcount.hpp"
#include "helpers/soft_patterns.hpp"

#define HANDLE_EARLY_RETURNS 1

void GenerateStacks::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	TRACE();
	MakeShared<Instance> s_fi, r_fi, s_fi2, r_fi2, s_fi3, r_fi3;
	MakeShared<Subroutine> s_func;
	MakeShared< MatchAll<Initialiser> > s_and;
	MakeShared<Compound> s_top_comp, r_top_comp, s_ret_comp, sn_ret_comp, r_ret_comp;
	MakeShared< Star<Declaration> > s_top_decls, r_top_decls, s_ret_decls, sn_ret_decls, r_ret_decls;
	MakeShared< Star<Statement> > s_top_pre, r_top_pre, s_ret_pre, s_ret_post, sn_ret_pre, sn_ret_post, r_ret_pre, r_ret_post;
	MakeShared< Stuff<Statement> > cs_stuff, s_stuff, r_stuff, s_stuff3, r_stuff3;
	MakeShared<Automatic> cs_instance, s_instance;
	MakeShared<Static> r_index, r_instance; // TODO Field
	MakeShared<Unsigned> r_index_type;
	MakeShared<PostIncrement> r_inc;
	MakeShared<PostDecrement> r_dec, sn_ret_dec, r_ret_dec;
	MakeShared<InstanceIdentifier> s_identifier;
	MakeShared<Array> r_array;
	MakeShared<Return> s_return, sn_return, r_return;
	MakeShared<Expression> ss_identifier;
	MakeShared<Subscript> sr_sub;
	MakeShared< NotMatch<Statement> > s_not3;
	MakeShared< MatchAll<Statement> > s_and3;

    // Master search - look for functions satisfying the construct limitation and get
	s_fi->identifier = MakeShared<InstanceIdentifier>();
	s_fi->type = s_func;
	s_fi->initialiser = s_and;
	s_top_comp->members = ( s_top_decls );
	s_top_comp->statements = ( s_top_pre );

	// Construct limitation - restrict master search to functions that contain an automatic variable
	cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

	// Master replace - insert index variable, inc and dec into function at top level
	r_fi->initialiser = r_top_comp;
	// top-level decls
	r_top_comp->members = ( r_top_decls, r_index );
	r_index->type = r_index_type;
	r_index_type->width = MakeShared<SpecificInteger>(32);
	MakeShared<SoftMakeIdentifier> r_index_identifier("%s_stack_index");
	r_index_identifier->source = MakeShared<Identifier>();
	r_index->identifier = r_index_identifier;
	r_index->constancy = MakeShared<NonConst>();
	r_index->initialiser = MakeShared<SpecificInteger>(0);
	// top-level statements
	r_inc->operands = ( MakeShared<InstanceIdentifier>() );
	r_top_comp->statements = ( r_inc, r_top_pre, r_dec );
	r_dec->operands = ( MakeShared<InstanceIdentifier>() );

    // Slave search to find automatic variables within the function
	s_fi2->identifier = MakeShared<InstanceIdentifier>();
	s_fi2->initialiser = s_stuff;
	s_stuff->terminus = s_instance;
	s_instance->identifier = s_identifier;
	s_instance->initialiser = MakeShared<Uninitialised>(); // can't handle initialisers!
	s_instance->type = MakeShared<Type>();

    // Slave replace to insert as a static array (TODO be a member of enclosing class)
	r_fi2->initialiser = r_stuff;
	r_instance->constancy = MakeShared<NonConst>();
	r_instance->initialiser = MakeShared<Uninitialised>();
	r_stuff->terminus = r_instance;
	MakeShared<SoftMakeIdentifier> r_identifier("%s_stack");
	r_identifier->source = MakeShared<Identifier>();
	r_instance->identifier = r_identifier;
	r_instance->type = r_array;
	r_array->element = MakeShared<Type>();
	r_array->size = MakeShared<SpecificInteger>(10);

	// Sub-slave replace with a subscript into the array
	sr_sub->operands = ( MakeShared<InstanceIdentifier>(), MakeShared<InstanceIdentifier>() );

#if HANDLE_EARLY_RETURNS
	// Slave to find early returns in the function
	s_fi3->identifier = MakeShared<InstanceIdentifier>();
	s_fi3->initialiser = s_stuff3;
	s_stuff3->terminus = s_and3;
	s_and3->patterns = ( s_ret_comp );
	s_ret_comp->members = ( s_ret_decls );
	s_ret_comp->statements = ( s_ret_pre, s_return, s_ret_post );
    s_and3->patterns = ( s_ret_comp, s_not3 );
	
    // Restrict the above to not include returns that come after an index decrement
    s_not3->pattern = sn_ret_comp;
	sn_ret_comp->members = ( sn_ret_decls );
	sn_ret_dec->operands = ( MakeShared<InstanceIdentifier>() );
	sn_ret_comp->statements = ( sn_ret_pre, sn_ret_dec, sn_return, sn_ret_post );

	// Slave replace with a decrement of the stack index coming before the return
	r_fi3->initialiser = r_stuff3;
	r_stuff3->terminus = r_ret_comp;
	r_ret_comp->members = ( r_ret_decls );
	r_ret_dec->operands = ( MakeShared<InstanceIdentifier>() );
	r_ret_comp->statements = ( r_ret_pre, r_ret_dec, r_return, r_ret_post );
#endif

	CouplingSet sms((
		Coupling(( s_top_decls, r_top_decls )), // Couple pre-existing decls in the function's top level
		Coupling(( s_top_pre, r_top_pre )), // Couple pre-existing statements in the funciton's top level
		Coupling(( s_stuff, r_stuff )), // Couple stuff between the function and the variable to be changed
		Coupling(( s_fi, r_fi )), // Couple the original function
		Coupling(( s_fi2, r_fi2 )), // Couple the function after master replace
		Coupling(( s_instance->type, r_array->element )), // Couple the type of the auto variable into the element type of the array
		Coupling(( s_identifier, r_identifier->source, ss_identifier )), // Couple the identifier of the auto variable for sub-slave and as source for array's name
		Coupling(( r_identifier, sr_sub->operands[0] )), // Couple the name of the array into the base of the subscript

#if HANDLE_EARLY_RETURNS
		Coupling(( s_fi->identifier, r_index_identifier->source, s_fi2->identifier, s_fi3->identifier )),
		Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->operands[1], r_ret_dec->operands[0], sn_ret_dec->operands[0] )),
		Coupling(( s_fi3, r_fi3 )), // Couple the function for dec-before-return slave
		Coupling(( s_stuff3, r_stuff3 )), // Couple stuff between function and compound containing return
		Coupling(( s_ret_decls, r_ret_decls )), // Couple decls in compound containing return
		Coupling(( s_ret_pre, r_ret_pre )), // Couple statements before return in compound
		Coupling(( s_return, r_return )), // Couple the return statement
		Coupling(( s_ret_post, sn_ret_post, r_ret_post )) ));// Couple statements after return in compound
                                                                          // make sure the ns and s are talking about the same return if there's more than one
#else
    	Coupling(( s_fi->identifier, r_index_identifier->source, s_fi2->identifier )),
        Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->index )) )); // Couple the name of the index variable into the index of the subscript and all the incs and decs
#endif

	vector<RootedSearchReplace *> vs;
	SearchReplace slave( ss_identifier, sr_sub );
	vs.push_back( &slave );
	SearchReplace mid( s_fi2, r_fi2, sms, vs );
	vector<RootedSearchReplace *> vs2;
	vs2.push_back( &mid );
#if HANDLE_EARLY_RETURNS
	SearchReplace ret( s_fi3, r_fi3 );
	vs2.push_back( &ret );
#endif
	SearchReplace( s_fi, r_fi, sms, vs2 )( context, proot );
}
