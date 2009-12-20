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

    // Master search - look for functions satisfying the construct limitation and get
	SharedNew<Instance> s_fi;
	s_fi->identifier = SharedNew<InstanceIdentifier>();
	SharedNew<Subroutine> s_func;
	s_fi->type = s_func;
	SharedNew< MatchAll<Initialiser> > s_and;
	SharedNew<Compound> s_top_comp;
	s_fi->initialiser = s_and;
	SharedNew< SearchReplace::Star<Declaration> > s_top_decls;
	s_top_comp->members = ( s_top_decls );
	SharedNew< SearchReplace::Star<Statement> > s_top_pre;
	s_top_comp->statements = ( s_top_pre );

	// Construct limitation - restrict master search to functions that contain an automatic variable
	SharedNew< SearchReplace::Stuff<Statement> > cs_stuff;
	SharedNew<Automatic> cs_instance;
	cs_stuff->terminus = cs_instance;
    s_and->patterns = ( s_top_comp, cs_stuff );

	// Master replace - insert index variable, inc and dec into function at top level
    SharedNew<Instance> r_fi;
    SharedNew<Compound> r_top_comp;
	r_fi->initialiser = r_top_comp;
	// top-level decls
	SharedNew< SearchReplace::Star<Declaration> > r_top_decls;
	SharedNew<Static> r_index;// TODO Field
	r_top_comp->members = ( r_top_decls, r_index );
	SharedNew<Unsigned> r_index_type;
	r_index->type = r_index_type;
	r_index_type->width = SharedPtr<SpecificInteger>( new SpecificInteger(32) );
	SharedPtr<SoftMakeIdentifier> r_index_identifier( new SoftMakeIdentifier("%s_stack_index") );
	r_index_identifier->source = SharedNew<Identifier>();
	r_index->identifier = r_index_identifier;
	r_index->constancy = SharedNew<NonConst>();
	r_index->initialiser = SharedPtr<SpecificInteger>( new SpecificInteger(0) );
	// top-level statements
	SharedNew<PostIncrement> r_inc;
	r_inc->operands = ( SharedNew<InstanceIdentifier>() );
	SharedNew< SearchReplace::Star<Statement> > r_top_pre;
	SharedNew<PostDecrement> r_dec;
	r_top_comp->statements = ( r_inc, r_top_pre, r_dec );
	r_dec->operands = ( SharedNew<InstanceIdentifier>() );

    // Slave search to find automatic variables within the function
	SharedNew<Instance> s_fi2;
	s_fi2->identifier = SharedNew<InstanceIdentifier>();
	SharedNew< SearchReplace::Stuff<Statement> > s_stuff;
	s_fi2->initialiser = s_stuff;
	SharedNew<Automatic> s_instance;
	s_stuff->terminus = s_instance;
	SharedNew<InstanceIdentifier> s_identifier;
	s_instance->identifier = s_identifier;
	s_instance->initialiser = SharedNew<Uninitialised>(); // can't handle initialisers!
	s_instance->type = SharedNew<Type>();

    // Slave replace to insert as a static array (TODO be a member of enclosing class)
	SharedNew<Instance> r_fi2;
	SharedNew< SearchReplace::Stuff<Statement> > r_stuff;
	r_fi2->initialiser = r_stuff;
	SharedNew<Static> r_instance; // TODO Field
	r_instance->constancy = SharedNew<NonConst>();
	r_instance->initialiser = SharedNew<Uninitialised>();
	r_stuff->terminus = r_instance;
	SharedPtr<SoftMakeIdentifier> r_identifier( new SoftMakeIdentifier("%s_stack") );
	r_identifier->source = SharedNew<Identifier>();
	r_instance->identifier = r_identifier;
	SharedNew<Array> r_array;
	r_instance->type = r_array;
	r_array->element = SharedNew<Type>();
	r_array->size = SharedPtr<SpecificInteger>( new SpecificInteger(10) );

	// Sub-slave to the above, find usages of the automatic variable
	SharedNew<Expression> ss_identifier;

	// Sub-slave replace with a subscript into the array
	SharedNew<Subscript> sr_sub;
	sr_sub->base = SharedNew<InstanceIdentifier>();
	sr_sub->index = SharedNew<InstanceIdentifier>();

#if HANDLE_EARLY_RETURNS
	// Slave to find early returns in the function
	SharedNew<Instance> s_fi3;
	s_fi3->identifier = SharedNew<InstanceIdentifier>();
	SharedNew< SearchReplace::Stuff<Statement> > s_stuff3;
	s_fi3->initialiser = s_stuff3;
	SharedNew< MatchAll<Statement> > s_and3;
	s_stuff3->terminus = s_and3;
	SharedNew<Compound> s_ret_comp;
	s_and3->patterns = ( s_ret_comp );
	SharedNew< SearchReplace::Star<Declaration> > s_ret_decls;
	s_ret_comp->members = ( s_ret_decls );
	SharedNew< SearchReplace::Star<Statement> > s_ret_pre;
	SharedNew<Return> s_return;
	SharedNew< SearchReplace::Star<Statement> > s_ret_post;
	s_ret_comp->statements = ( s_ret_pre, s_return, s_ret_post );
	SharedNew< NotMatch<Statement> > s_not3;
    s_and3->patterns = ( s_ret_comp, s_not3 );
	
    // Restrict the above to not include returns that come after an index decrement
    SharedNew<Compound> sn_ret_comp;
    s_not3->pattern = sn_ret_comp;
    SharedNew< SearchReplace::Star<Declaration> > sn_ret_decls;
	sn_ret_comp->members = ( sn_ret_decls );
	SharedNew< SearchReplace::Star<Statement> > sn_ret_pre;
	SharedNew<PostDecrement> sn_ret_dec;
	sn_ret_dec->operands = ( SharedNew<InstanceIdentifier>() );
	SharedNew<Return> sn_return;
	SharedNew< SearchReplace::Star<Statement> > sn_ret_post;
	sn_ret_comp->statements = ( sn_ret_pre, sn_ret_dec, sn_return, sn_ret_post );

	// Slave replace with a decrement of the stack index coming before the return
	SharedNew<Instance> r_fi3;
	SharedNew< SearchReplace::Stuff<Statement> > r_stuff3;
	r_fi3->initialiser = r_stuff3;
	SharedNew<Compound> r_ret_comp;
	r_stuff3->terminus = r_ret_comp;
	SharedNew< SearchReplace::Star<Declaration> > r_ret_decls;
	r_ret_comp->members = ( r_ret_decls );
	SharedNew< SearchReplace::Star<Statement> > r_ret_pre;
	SharedNew<PostDecrement> r_ret_dec;
	r_ret_dec->operands = ( SharedNew<InstanceIdentifier>() );
	SharedNew<Return> r_return;
	SharedNew< SearchReplace::Star<Statement> > r_ret_post;
	r_ret_comp->statements = ( r_ret_pre, r_ret_dec, r_return, r_ret_post );
#endif

	SearchReplace::CouplingSet sms((
		SearchReplace::Coupling(( s_top_decls, r_top_decls )), // Couple pre-existing decls in the function's top level
		SearchReplace::Coupling(( s_top_pre, r_top_pre )), // Couple pre-existing statements in the funciton's top level
		SearchReplace::Coupling(( s_stuff, r_stuff )), // Couple stuff between the function and the variable to be changed
		SearchReplace::Coupling(( s_fi, r_fi )), // Couple the original function
		SearchReplace::Coupling(( s_fi2, r_fi2 )), // Couple the function after master replace
		SearchReplace::Coupling(( s_instance->type, r_array->element )), // Couple the type of the auto variable into the element type of the array
		SearchReplace::Coupling(( s_identifier, r_identifier->source, ss_identifier )), // Couple the identifier of the auto variable for sub-slave and as source for array's name
		SearchReplace::Coupling(( r_identifier, sr_sub->base )), // Couple the name of the array into the base of the subscript

#if HANDLE_EARLY_RETURNS
		SearchReplace::Coupling(( s_fi->identifier, r_index_identifier->source, s_fi2->identifier, s_fi3->identifier )),
		SearchReplace::Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->index, r_ret_dec->operands[0], sn_ret_dec->operands[0] )),
		SearchReplace::Coupling(( s_fi3, r_fi3 )), // Couple the function for dec-before-return slave
		SearchReplace::Coupling(( s_stuff3, r_stuff3 )), // Couple stuff between function and compound containing return
		SearchReplace::Coupling(( s_ret_decls, r_ret_decls )), // Couple decls in compound containing return
		SearchReplace::Coupling(( s_ret_pre, r_ret_pre )), // Couple statements before return in compound
		SearchReplace::Coupling(( s_return, r_return )), // Couple the return statement
		SearchReplace::Coupling(( s_ret_post, sn_ret_post, r_ret_post )) ));// Couple statements after return in compound
                                                                          // make sure the ns and s are talking about the same return if there's more than one
#else
    	SearchReplace::Coupling(( s_fi->identifier, r_index_identifier->source, s_fi2->identifier )),
        SearchReplace::Coupling(( r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->index )) )); // Couple the name of the index variable into the index of the subscript and all the incs and decs
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
