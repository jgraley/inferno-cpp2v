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


void GenerateStacks::operator()( shared_ptr<Program> program )
{
	TRACE();
	set<SearchReplace::MatchSet *> sms;

	shared_ptr<Instance> s_fi( new Instance );
	shared_ptr<Subroutine> s_func( new Subroutine );
	s_fi->type = s_func;
    shared_ptr<Compound> s_top_comp( new Compound );
	s_fi->initialiser = s_top_comp;
	shared_ptr< SearchReplace::Star<Declaration> > s_top_decls( new SearchReplace::Star<Declaration> );
	s_top_comp->members.insert( s_top_decls );
	shared_ptr< SearchReplace::Star<Statement> > s_top_pre( new SearchReplace::Star<Statement> );
	s_top_comp->statements.push_back( s_top_pre );
	shared_ptr< SearchReplace::Stuff<Statement> > s_stuff( new SearchReplace::Stuff<Statement> );
	s_top_comp->statements.push_back( s_stuff );
	shared_ptr< SearchReplace::Star<Statement> > s_top_post( new SearchReplace::Star<Statement> );
	s_top_comp->statements.push_back( s_top_post );

	shared_ptr<Instance> s_instance( new Instance );
	s_stuff->terminus = s_instance;
	shared_ptr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
	s_instance->identifier = s_identifier;
	s_instance->storage = shared_new<Auto>();
	s_instance->type = shared_new<Type>();


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
	shared_ptr< SearchReplace::Stuff<Statement> > r_stuff( new SearchReplace::Stuff<Statement> );
	r_top_comp->statements.push_back( r_stuff );
	shared_ptr< SearchReplace::Star<Statement> > r_top_post( new SearchReplace::Star<Statement> );
	r_top_comp->statements.push_back( r_top_post );

	// under "stuff"
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
	shared_ptr<PostDecrement> r_dec( new PostDecrement );
	r_top_comp->statements.push_back( r_dec );
	r_dec->operands.push_back( shared_new<InstanceIdentifier>() );

	shared_ptr<Identifier> ss_identifier( new Identifier );

	shared_ptr<Subscript> sr_sub( new Subscript );
	sr_sub->base = shared_new<InstanceIdentifier>();
	sr_sub->index = shared_new<InstanceIdentifier>();

	SearchReplace::MatchSet ms_top_decls;
	ms_top_decls.insert( s_top_decls );
	ms_top_decls.insert( r_top_decls );
	sms.insert( &ms_top_decls );

	SearchReplace::MatchSet ms_top_pre;
	ms_top_pre.insert( s_top_pre );
	ms_top_pre.insert( r_top_pre );
	sms.insert( &ms_top_pre );

	SearchReplace::MatchSet ms_stuff;
	ms_stuff.insert( s_stuff );
	ms_stuff.insert( r_stuff );
	sms.insert( &ms_stuff );

	SearchReplace::MatchSet ms_top_post;
	ms_top_post.insert( s_top_post );
	ms_top_post.insert( r_top_post );
	sms.insert( &ms_top_post );

	SearchReplace::MatchSet ms_fi;
	ms_fi.insert( s_fi );
	ms_fi.insert( r_fi );
	sms.insert( &ms_fi );

	SearchReplace::MatchSet ms_instance;
	ms_instance.insert( s_instance );
	ms_instance.insert( r_instance );
	sms.insert( &ms_instance );

	SearchReplace::MatchSet ms_type;
	ms_type.insert( s_instance->type );
	ms_type.insert( r_array->element );
	sms.insert( &ms_type );

	SearchReplace::MatchSet ms_identifier;
	ms_identifier.insert( s_identifier );
	ms_identifier.insert( r_identifier->source );
	ms_identifier.insert( r_index_identifier->source );
	ms_identifier.insert( ss_identifier );
	sms.insert( &ms_identifier );

	SearchReplace::MatchSet ms_new_identifier;
	ms_new_identifier.insert( r_identifier );
	ms_new_identifier.insert( sr_sub->base );
	sms.insert( &ms_new_identifier );

	SearchReplace::MatchSet ms_new_index_identifier;
	ms_new_index_identifier.insert( r_index_identifier );
	ms_new_index_identifier.insert( r_inc->operands[0] );
	ms_new_index_identifier.insert( r_dec->operands[0] );
	ms_new_index_identifier.insert( sr_sub->index );
	sms.insert( &ms_new_index_identifier );

	vector<RootedSearchReplace *> vs;
	SearchReplace slave( ss_identifier, sr_sub );
	vs.push_back( &slave );
	SearchReplace( s_fi, r_fi, sms, vs )( program );

	// TODO insert dec before return statements
}
