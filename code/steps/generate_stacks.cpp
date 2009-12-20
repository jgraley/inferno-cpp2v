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
	SearchReplace::CouplingSet sms;

    // Master search - look for functions satisfying the construct limitation and get
	SharedPtr<Instance> s_fi( new Instance );
	s_fi->identifier = shared_new<InstanceIdentifier>();
	SharedPtr<Subroutine> s_func( new Subroutine );
	s_fi->type = s_func;
    SharedPtr< MatchAll<Initialiser> > s_and( new MatchAll<Initialiser> );
    SharedPtr<Compound> s_top_comp( new Compound );
    s_and->patterns.insert( s_top_comp );
	s_fi->initialiser = s_and;
	SharedPtr< SearchReplace::Star<Declaration> > s_top_decls( new SearchReplace::Star<Declaration> );
	s_top_comp->members.insert( s_top_decls );
	SharedPtr< SearchReplace::Star<Statement> > s_top_pre( new SearchReplace::Star<Statement> );
	s_top_comp->statements.push_back( s_top_pre );

	// Construct limitation - restrict master search to functions that contain an automatic variable
	SharedPtr< SearchReplace::Stuff<Statement> > cs_stuff( new SearchReplace::Stuff<Statement> );
	SharedPtr<Automatic> cs_instance( new Automatic );
	cs_stuff->terminus = cs_instance;
    s_and->patterns.insert( cs_stuff );

	// Master replace - insert index variable, inc and dec into function at top level
	SharedPtr<Instance> r_fi( new Instance );
    SharedPtr<Compound> r_top_comp( new Compound );
	r_fi->initialiser = r_top_comp;
	// top-level decls
	SharedPtr< SearchReplace::Star<Declaration> > r_top_decls( new SearchReplace::Star<Declaration> );
	r_top_comp->members.insert( r_top_decls );
	SharedPtr<Static> r_index( new Static );// TODO Field
	r_top_comp->members.insert( r_index );
	SharedPtr<Unsigned> r_index_type( new Unsigned );
	r_index->type = r_index_type;
	r_index_type->width = SharedPtr<SpecificInteger>( new SpecificInteger(32) );
	SharedPtr<SoftMakeIdentifier> r_index_identifier( new SoftMakeIdentifier("%s_stack_index") );
	r_index_identifier->source = shared_new<Identifier>();
	r_index->identifier = r_index_identifier;
	r_index->constancy = shared_new<NonConst>();
	r_index->initialiser = SharedPtr<SpecificInteger>( new SpecificInteger(0) );
	// top-level statements
	SharedPtr<PostIncrement> r_inc( new PostIncrement );
	r_top_comp->statements.push_back( r_inc );
	r_inc->operands.push_back( shared_new<InstanceIdentifier>() );
	SharedPtr< SearchReplace::Star<Statement> > r_top_pre( new SearchReplace::Star<Statement> );
	r_top_comp->statements.push_back( r_top_pre );
	SharedPtr<PostDecrement> r_dec( new PostDecrement );
	r_top_comp->statements.push_back( r_dec );
	r_dec->operands.push_back( shared_new<InstanceIdentifier>() );

    // Slave search to find automatic variables within the function
	SharedPtr<Instance> s_fi2( new Instance );
	s_fi2->identifier = shared_new<InstanceIdentifier>();
	SharedPtr< SearchReplace::Stuff<Statement> > s_stuff( new SearchReplace::Stuff<Statement> );
	s_fi2->initialiser = s_stuff;
	SharedPtr<Automatic> s_instance( new Automatic );
	s_stuff->terminus = s_instance;
	SharedPtr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
	s_instance->identifier = s_identifier;
	s_instance->initialiser = shared_new<Uninitialised>(); // can't handle initialisers!
	s_instance->type = shared_new<Type>();

    // Slave replace to insert as a static array (TODO be a member of enclosing class)
	SharedPtr<Instance> r_fi2( new Instance );
	SharedPtr< SearchReplace::Stuff<Statement> > r_stuff( new SearchReplace::Stuff<Statement> );
	r_fi2->initialiser = r_stuff;
	SharedPtr<Static> r_instance( new Static ); // TODO Field
	r_instance->constancy = shared_new<NonConst>();
	r_instance->initialiser = shared_new<Uninitialised>();
	r_stuff->terminus = r_instance;
	SharedPtr<SoftMakeIdentifier> r_identifier( new SoftMakeIdentifier("%s_stack") );
	r_identifier->source = shared_new<Identifier>();
	r_instance->identifier = r_identifier;
	SharedPtr<Array> r_array( new Array );
	r_instance->type = r_array;
	r_array->element = shared_new<Type>();
	r_array->size = SharedPtr<SpecificInteger>( new SpecificInteger(10) );

	// Sub-slave to the above, find usages of the automatic variable
	SharedPtr<Expression> ss_identifier( new Expression );

	// Sub-slave replace with a subscript into the array
	SharedPtr<Subscript> sr_sub( new Subscript );
	sr_sub->base = shared_new<InstanceIdentifier>();
	sr_sub->index = shared_new<InstanceIdentifier>();

#if HANDLE_EARLY_RETURNS
	// Slave to find early returns in the function
	SharedPtr<Instance> s_fi3( new Instance );
	s_fi3->identifier = shared_new<InstanceIdentifier>();
	SharedPtr< SearchReplace::Stuff<Statement> > s_stuff3( new SearchReplace::Stuff<Statement> );
	s_fi3->initialiser = s_stuff3;
	SharedPtr< MatchAll<Statement> > s_and3( new MatchAll<Statement> );
	s_stuff3->terminus = s_and3;
	SharedPtr<Compound> s_ret_comp( new Compound );
	s_and3->patterns.insert( s_ret_comp );
	SharedPtr< SearchReplace::Star<Declaration> > s_ret_decls( new SearchReplace::Star<Declaration> );
	s_ret_comp->members.insert( s_ret_decls );	
	SharedPtr< SearchReplace::Star<Statement> > s_ret_pre( new SearchReplace::Star<Statement> );
	s_ret_comp->statements.push_back( s_ret_pre );
	SharedPtr<Return> s_return( new Return );
	s_ret_comp->statements.push_back( s_return );
	SharedPtr< SearchReplace::Star<Statement> > s_ret_post( new SearchReplace::Star<Statement> );
	s_ret_comp->statements.push_back( s_ret_post );
    SharedPtr< NotMatch<Statement> > s_not3( new NotMatch<Statement> );    
    s_and3->patterns.insert( s_not3 );
	
    // Restrict the above to not include returns that come after an index decrement
	SharedPtr<Compound> sn_ret_comp( new Compound );
    s_not3->pattern = sn_ret_comp;
	SharedPtr< SearchReplace::Star<Declaration> > sn_ret_decls( new SearchReplace::Star<Declaration> );
	sn_ret_comp->members.insert( sn_ret_decls );	
	SharedPtr< SearchReplace::Star<Statement> > sn_ret_pre( new SearchReplace::Star<Statement> );
	sn_ret_comp->statements.push_back( sn_ret_pre );
	SharedPtr<PostDecrement> sn_ret_dec( new PostDecrement );
	sn_ret_dec->operands.push_back( shared_new<InstanceIdentifier>() );
	sn_ret_comp->statements.push_back( sn_ret_dec );
	SharedPtr<Return> sn_return( new Return );
	sn_ret_comp->statements.push_back( sn_return );
	SharedPtr< SearchReplace::Star<Statement> > sn_ret_post( new SearchReplace::Star<Statement> );
	sn_ret_comp->statements.push_back( sn_ret_post );

	// Slave replace with a decrement of the stack index coming before the return
	SharedPtr<Instance> r_fi3( new Instance );
	SharedPtr< SearchReplace::Stuff<Statement> > r_stuff3( new SearchReplace::Stuff<Statement> );
	r_fi3->initialiser = r_stuff3;
	SharedPtr<Compound> r_ret_comp( new Compound );
	r_stuff3->terminus = r_ret_comp;
	SharedPtr< SearchReplace::Star<Declaration> > r_ret_decls( new SearchReplace::Star<Declaration> );
	r_ret_comp->members.insert( r_ret_decls );	
	SharedPtr< SearchReplace::Star<Statement> > r_ret_pre( new SearchReplace::Star<Statement> );
	r_ret_comp->statements.push_back( r_ret_pre );
	SharedPtr<PostDecrement> r_ret_dec( new PostDecrement );
	r_ret_comp->statements.push_back( r_ret_dec );
	r_ret_dec->operands.push_back( shared_new<InstanceIdentifier>() );
	SharedPtr<Return> r_return( new Return );
	r_ret_comp->statements.push_back( r_return );
	SharedPtr< SearchReplace::Star<Statement> > r_ret_post( new SearchReplace::Star<Statement> );
	r_ret_comp->statements.push_back( r_ret_post );
#endif

	// Couple pre-existing decls in the function's top level
	SearchReplace::Coupling ms_top_decls((s_top_decls, r_top_decls));
	sms.insert( ms_top_decls );

	// Couple pre-existing statements in the funciton's top level
	SearchReplace::Coupling ms_top_pre((s_top_pre, r_top_pre));
	sms.insert( ms_top_pre );

	// Couple stuff between the function and the variable to be changed
	SearchReplace::Coupling ms_stuff((s_stuff, r_stuff));
	sms.insert( ms_stuff );

	// Couple the original function
	SearchReplace::Coupling ms_fi((s_fi, r_fi));
	sms.insert( ms_fi );

	// Couple the function after master replace
	SearchReplace::Coupling ms_fi2((s_fi2, r_fi2));
	sms.insert( ms_fi2 );

	// Couple the type of the auto variable into the element type of the array
	SearchReplace::Coupling ms_type((s_instance->type, r_array->element));
	sms.insert( ms_type );

	// Couple the identifier of the auto variable for sub-slave and as source for array's name
	SearchReplace::Coupling ms_identifier((s_identifier, r_identifier->source, ss_identifier));
	sms.insert( ms_identifier );

	// Couple the name of the function into slaves and as source for index variable's name
	SearchReplace::Coupling ms_function_identifier((s_fi->identifier, r_index_identifier->source, s_fi2->identifier));
#if HANDLE_EARLY_RETURNS
	ms_function_identifier.insert( s_fi3->identifier );
#endif
	sms.insert( ms_function_identifier );

	// Couple the name of the array into the base of the subscript
	SearchReplace::Coupling ms_new_identifier((r_identifier, sr_sub->base));
	sms.insert( ms_new_identifier );

	// Couple the name of the index variable into the index of the subscript and all the incs and decs
	SearchReplace::Coupling ms_new_index_identifier((r_index_identifier, r_inc->operands[0], r_dec->operands[0], sr_sub->index));
#if HANDLE_EARLY_RETURNS
	ms_new_index_identifier.insert( r_ret_dec->operands[0] );
	ms_new_index_identifier.insert( sn_ret_dec->operands[0] );
#endif
	sms.insert( ms_new_index_identifier );

#if HANDLE_EARLY_RETURNS
	// Couple the function for dec-before-return slave
	SearchReplace::Coupling ms_fi3((s_fi3, r_fi3));
	sms.insert( ms_fi3 );

	// Couple stuff between function and compound containing return
	SearchReplace::Coupling ms_stuff3((s_stuff3, r_stuff3));
	sms.insert( ms_stuff3 );

	// Couple decls in compound containing return
	SearchReplace::Coupling ms_ret_decls((s_ret_decls, r_ret_decls));
	sms.insert( ms_ret_decls );

	// Couple statements before return in compound
	SearchReplace::Coupling ms_ret_pre((s_ret_pre, r_ret_pre));
	sms.insert( ms_ret_pre );

	// Couple the return statement
	SearchReplace::Coupling ms_return((s_return, r_return));
	sms.insert( ms_return );

	// Couple statements after return in compound
	// make sure the ns and s are talking about the same return if there's more than one
	SearchReplace::Coupling ms_ret_post((s_ret_post, sn_ret_post, r_ret_post));
	sms.insert( ms_ret_post );
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
