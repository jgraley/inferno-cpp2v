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


void GenerateStacks::operator()( shared_ptr<Node> context, shared_ptr<Node> *proot )
{
	TRACE();
	set<SearchReplace::Coupling *> sms;

    // Master search - look for functions satisfying the construct limitation and get
	shared_ptr<Instance> s_fi( new Instance );
	s_fi->identifier = shared_new<InstanceIdentifier>();
	shared_ptr<Subroutine> s_func( new Subroutine );
	s_fi->type = s_func;
    shared_ptr< SoftAnd<Initialiser> > s_and( new SoftAnd<Initialiser> );
    shared_ptr<Compound> s_top_comp( new Compound );
    s_and->patterns.insert( s_top_comp );
	s_fi->initialiser = s_and;
	shared_ptr< SearchReplace::Star<Declaration> > s_top_decls( new SearchReplace::Star<Declaration> );
	s_top_comp->members.insert( s_top_decls );
	shared_ptr< SearchReplace::Star<Statement> > s_top_pre( new SearchReplace::Star<Statement> );
	s_top_comp->statements.push_back( s_top_pre );

	// Construct limitation - restrict master search to functions that contain an automatic variable
	shared_ptr< SearchReplace::Stuff<Statement> > cs_stuff( new SearchReplace::Stuff<Statement> );
	shared_ptr<Instance> cs_instance( new Instance );
	cs_stuff->terminus = cs_instance;
	cs_instance->storage = shared_new<Auto>();
    s_and->patterns.insert( cs_stuff );

	// Master replace - insert index variable, inc and dec into function at top level
	shared_ptr<Instance> r_fi( new Instance );
    shared_ptr<Compound> r_top_comp( new Compound );
	r_fi->initialiser = r_top_comp;
	// top-level decls
	shared_ptr< SearchReplace::Star<Declaration> > r_top_decls( new SearchReplace::Star<Declaration> );
	r_top_comp->members.insert( r_top_decls );
	shared_ptr<Instance> r_index( new Instance );
	r_top_comp->members.insert( r_index );
	shared_ptr<Unsigned> r_index_type( new Unsigned );
	r_index->type = r_index_type;
	r_index_type->width = shared_ptr<SpecificInteger>( new SpecificInteger(32) );
	shared_ptr<SoftMakeIdentifier> r_index_identifier( new SoftMakeIdentifier("%s_stack_index") );
	r_index_identifier->source = shared_new<Identifier>();
	r_index->identifier = r_index_identifier;
	r_index->storage = shared_new<Static>(); // TODO Member
	r_index->constancy = shared_new<NonConst>();
	r_index->initialiser = shared_ptr<SpecificInteger>( new SpecificInteger(0) );
	r_index->access = shared_new<Private>();
	// top-level statements
	shared_ptr<PostIncrement> r_inc( new PostIncrement );
	r_top_comp->statements.push_back( r_inc );
	r_inc->operands.push_back( shared_new<InstanceIdentifier>() );
	shared_ptr< SearchReplace::Star<Statement> > r_top_pre( new SearchReplace::Star<Statement> );
	r_top_comp->statements.push_back( r_top_pre );
	shared_ptr<PostDecrement> r_dec( new PostDecrement );
	r_top_comp->statements.push_back( r_dec );
	r_dec->operands.push_back( shared_new<InstanceIdentifier>() );

    // Slave search to find automatic variables within the function
	shared_ptr<Instance> s_fi2( new Instance );
	s_fi2->identifier = shared_new<InstanceIdentifier>();
	shared_ptr< SearchReplace::Stuff<Statement> > s_stuff( new SearchReplace::Stuff<Statement> );
	s_fi2->initialiser = s_stuff;
	shared_ptr<Instance> s_instance( new Instance );
	s_stuff->terminus = s_instance;
	shared_ptr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
	s_instance->identifier = s_identifier;
	s_instance->storage = shared_new<Auto>();
	s_instance->type = shared_new<Type>();

    // Slave replace to insert as a static array (TODO be a member of enclosing class)
	shared_ptr<Instance> r_fi2( new Instance );
	shared_ptr< SearchReplace::Stuff<Statement> > r_stuff( new SearchReplace::Stuff<Statement> );
	r_fi2->initialiser = r_stuff;
	shared_ptr<Instance> r_instance( new Instance );
	r_stuff->terminus = r_instance;
	shared_ptr<SoftMakeIdentifier> r_identifier( new SoftMakeIdentifier("%s_stack") );
	r_identifier->source = shared_new<Identifier>();
	r_instance->identifier = r_identifier;
	r_instance->storage = shared_new<Static>(); // TODO Member
	shared_ptr<Array> r_array( new Array );
	r_instance->type = r_array;
	r_array->element = shared_new<Type>();
	r_array->size = shared_ptr<SpecificInteger>( new SpecificInteger(10) );

	// Sub-slave to the above, find usages of the automatic variable
	shared_ptr<Expression> ss_identifier( new Expression );

	// Sub-slave replace with a subscript into the array
	shared_ptr<Subscript> sr_sub( new Subscript );
	sr_sub->base = shared_new<InstanceIdentifier>();
	sr_sub->index = shared_new<InstanceIdentifier>();

	// Slave to find early returns in the function
	shared_ptr<Instance> s_fi3( new Instance );
	s_fi3->identifier = shared_new<InstanceIdentifier>();
	shared_ptr< SearchReplace::Stuff<Statement> > s_stuff3( new SearchReplace::Stuff<Statement> );
	s_fi3->initialiser = s_stuff3;
	shared_ptr< SoftAnd<Statement> > s_and3( new SoftAnd<Statement> );
	s_stuff3->terminus = s_and3;
	shared_ptr<Compound> s_ret_comp( new Compound );
	s_and3->patterns.insert( s_ret_comp );
	shared_ptr< SearchReplace::Star<Declaration> > s_ret_decls( new SearchReplace::Star<Declaration> );
	s_ret_comp->members.insert( s_ret_decls );	
	shared_ptr< SearchReplace::Star<Statement> > s_ret_pre( new SearchReplace::Star<Statement> );
	s_ret_comp->statements.push_back( s_ret_pre );
	shared_ptr<Return> s_return( new Return );
	s_ret_comp->statements.push_back( s_return );
	shared_ptr< SearchReplace::Star<Statement> > s_ret_post( new SearchReplace::Star<Statement> );
	s_ret_comp->statements.push_back( s_ret_post );
    shared_ptr< SoftNot<Statement> > s_not3( new SoftNot<Statement> );    
    s_and3->patterns.insert( s_not3 );
	
    // Restrict the above to not include returns that come after an index decrement
	shared_ptr<Compound> sn_ret_comp( new Compound );
    s_not3->pattern = sn_ret_comp;
	shared_ptr< SearchReplace::Star<Declaration> > sn_ret_decls( new SearchReplace::Star<Declaration> );
	sn_ret_comp->members.insert( sn_ret_decls );	
	shared_ptr< SearchReplace::Star<Statement> > sn_ret_pre( new SearchReplace::Star<Statement> );
	sn_ret_comp->statements.push_back( sn_ret_pre );
	shared_ptr<PostDecrement> sn_ret_dec( new PostDecrement );
	sn_ret_dec->operands.push_back( shared_new<InstanceIdentifier>() );
	sn_ret_comp->statements.push_back( sn_ret_dec );
	shared_ptr<Return> sn_return( new Return );
	sn_ret_comp->statements.push_back( sn_return );
	shared_ptr< SearchReplace::Star<Statement> > sn_ret_post( new SearchReplace::Star<Statement> );
	sn_ret_comp->statements.push_back( sn_ret_post );

	// Slave replace with a decrement of the stack index coming before the return
	shared_ptr<Instance> r_fi3( new Instance );
	shared_ptr< SearchReplace::Stuff<Statement> > r_stuff3( new SearchReplace::Stuff<Statement> );
	r_fi3->initialiser = r_stuff3;
	shared_ptr<Compound> r_ret_comp( new Compound );
	r_stuff3->terminus = r_ret_comp;
	shared_ptr< SearchReplace::Star<Declaration> > r_ret_decls( new SearchReplace::Star<Declaration> );
	r_ret_comp->members.insert( r_ret_decls );	
	shared_ptr< SearchReplace::Star<Statement> > r_ret_pre( new SearchReplace::Star<Statement> );
	r_ret_comp->statements.push_back( r_ret_pre );
	shared_ptr<PostDecrement> r_ret_dec( new PostDecrement );
	r_ret_comp->statements.push_back( r_ret_dec );
	r_ret_dec->operands.push_back( shared_new<InstanceIdentifier>() );
	shared_ptr<Return> r_return( new Return );
	r_ret_comp->statements.push_back( r_return );
	shared_ptr< SearchReplace::Star<Statement> > r_ret_post( new SearchReplace::Star<Statement> );
	r_ret_comp->statements.push_back( r_ret_post );

	// Couple pre-existing decls in the function's top level
	SearchReplace::Coupling ms_top_decls;
	ms_top_decls.insert( s_top_decls );
	ms_top_decls.insert( r_top_decls );
	sms.insert( &ms_top_decls );

	// Couple pre-existing statements in the funciton's top level
	SearchReplace::Coupling ms_top_pre;
	ms_top_pre.insert( s_top_pre );
	ms_top_pre.insert( r_top_pre );
	sms.insert( &ms_top_pre );

	// Couple stuff between the function and the variable to be changed
	SearchReplace::Coupling ms_stuff;
	//ms_stuff.insert( s_top_comp );
	ms_stuff.insert( s_stuff );
	ms_stuff.insert( r_stuff );
	sms.insert( &ms_stuff );

	// Couple the original function
	SearchReplace::Coupling ms_fi;
	ms_fi.insert( s_fi );
	ms_fi.insert( r_fi );
	sms.insert( &ms_fi );

	// Couple the function after master replace
	SearchReplace::Coupling ms_fi2;
	ms_fi2.insert( s_fi2 );
	ms_fi2.insert( r_fi2 );
	sms.insert( &ms_fi2 );

	// Couple the automatic variable that will become an array
	SearchReplace::Coupling ms_instance;
	ms_instance.insert( s_instance );
	ms_instance.insert( r_instance );
	sms.insert( &ms_instance );

	// Couple the type of the auto variable into the element type of the array
	SearchReplace::Coupling ms_type;
	ms_type.insert( s_instance->type );
	ms_type.insert( r_array->element );
	sms.insert( &ms_type );

	// Couple the identifier of the auto variable for sub-slave and as source for array's name
	SearchReplace::Coupling ms_identifier;
	ms_identifier.insert( s_identifier );
	ms_identifier.insert( r_identifier->source );
	ms_identifier.insert( ss_identifier );
	sms.insert( &ms_identifier );

	// Couple the name of the function into slaves and as source for index variable's name
	SearchReplace::Coupling ms_function_identifier;
	ms_function_identifier.insert( s_fi->identifier );
	ms_function_identifier.insert( r_index_identifier->source );
	ms_function_identifier.insert( s_fi2->identifier );
	ms_function_identifier.insert( s_fi3->identifier );
	sms.insert( &ms_function_identifier );

	// Couple the name of the array into the base of the subscript
	SearchReplace::Coupling ms_new_identifier;
	ms_new_identifier.insert( r_identifier );
	ms_new_identifier.insert( sr_sub->base );
	sms.insert( &ms_new_identifier );

	// Couple the name of the index variable into the index of the subscript and all the incs and decs
	SearchReplace::Coupling ms_new_index_identifier;
	ms_new_index_identifier.insert( r_index_identifier );
	ms_new_index_identifier.insert( r_inc->operands[0] );
	ms_new_index_identifier.insert( r_dec->operands[0] );
	ms_new_index_identifier.insert( sr_sub->index );
	ms_new_index_identifier.insert( r_ret_dec->operands[0] );
	ms_new_index_identifier.insert( sn_ret_dec->operands[0] );
	sms.insert( &ms_new_index_identifier );

	// Couple the function for dec-before-return slave
	SearchReplace::Coupling ms_fi3;
	ms_fi3.insert( s_fi3 );
	ms_fi3.insert( r_fi3 );
	sms.insert( &ms_fi3 );

	// Couple stuff between function and compound containing return
	SearchReplace::Coupling ms_stuff3;
	ms_stuff3.insert( s_stuff3 );
	ms_stuff3.insert( r_stuff3 );
	sms.insert( &ms_stuff3 );

	// Couple decls in compound containing return
	SearchReplace::Coupling ms_ret_decls;
	ms_ret_decls.insert( s_ret_decls );
	ms_ret_decls.insert( r_ret_decls );
	sms.insert( &ms_ret_decls );

	// Couple statements before return in compound
	SearchReplace::Coupling ms_ret_pre;
	ms_ret_pre.insert( s_ret_pre );
	ms_ret_pre.insert( r_ret_pre );
	sms.insert( &ms_ret_pre );

	// Couple the return statement
	SearchReplace::Coupling ms_return;
	ms_return.insert( s_return );
	//ms_return.insert( sn_return );
	ms_return.insert( r_return );
	sms.insert( &ms_return );

	// Couple statements after return in compound
	SearchReplace::Coupling ms_ret_post;
	ms_ret_post.insert( s_ret_post );
	ms_ret_post.insert( sn_ret_post ); // make sure the ns and s are talking about the same return if there's more than one
	ms_ret_post.insert( r_ret_post );
	sms.insert( &ms_ret_post );


	vector<RootedSearchReplace *> vs;
	SearchReplace slave( ss_identifier, sr_sub );
	vs.push_back( &slave );
	SearchReplace mid( s_fi2, r_fi2, sms, vs );
	SearchReplace ret( s_fi3, r_fi3 );
	vector<RootedSearchReplace *> vs2;
	vs2.push_back( &mid );
	vs2.push_back( &ret );
	SearchReplace( s_fi, r_fi, sms, vs2 )( context, proot );
}
