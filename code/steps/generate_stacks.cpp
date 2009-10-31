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
	//while(1)
	{
		TRACE();
		set<SearchReplace::MatchSet *> sms;

		shared_ptr<Instance> s_fi( new Instance );
		shared_ptr<Subroutine> s_func( new Subroutine );
		s_fi->type = s_func;
		shared_ptr< SearchReplace::Stuff<Statement> > s_stuff( new SearchReplace::Stuff<Statement> );
		s_fi->initialiser = s_stuff;
		shared_ptr<Compound> s_comp( new Compound );
		s_stuff->terminus = s_comp;
		shared_ptr<Instance> s_instance( new Instance );
		s_comp->members.insert( s_instance );
		shared_ptr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
		s_instance->identifier = s_identifier;
		s_instance->storage = shared_new<Auto>();
		s_instance->type = shared_new<Type>();
		shared_ptr< SearchReplace::Star<Declaration> > s_other_decls( new SearchReplace::Star<Declaration> );
		s_comp->members.insert( s_other_decls );
		s_comp->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );

		shared_ptr<Instance> r_fi( new Instance );
		shared_ptr< SearchReplace::Stuff<Statement> > r_stuff( new SearchReplace::Stuff<Statement> );
		r_fi->initialiser = r_stuff;
		shared_ptr<Compound> r_comp( new Compound );
		r_stuff->terminus = r_comp;
		shared_ptr<Instance> r_instance( new Instance );
		r_comp->members.insert( r_instance );
		shared_ptr<SoftMakeIdentifier> r_identifier( new SoftMakeIdentifier("%s_stack") );
		r_identifier->source = shared_new<Identifier>();
		r_instance->identifier = r_identifier;
		r_instance->storage = shared_new<Static>(); // TODO Member
		shared_ptr<Array> r_array( new Array );
		r_instance->type = r_array;
		r_array->element = shared_new<Type>();
		r_array->size = shared_ptr<SpecificInteger>( new SpecificInteger(10) );
		shared_ptr<Instance> r_index( new Instance );
		r_comp->members.insert( r_index );
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
		shared_ptr< SearchReplace::Star<Declaration> > r_other_decls( new SearchReplace::Star<Declaration> );
		r_comp->members.insert( r_other_decls );
		shared_ptr<PostIncrement> r_inc( new PostIncrement );
		r_comp->statements.push_back( r_inc );
		r_inc->operands.push_back( r_index_identifier );
		r_comp->statements.push_back( shared_new< SearchReplace::Star<Statement> >() );
		shared_ptr<PostDecrement> r_dec( new PostDecrement );
		r_comp->statements.push_back( r_dec );
		r_dec->operands.push_back( r_index_identifier );

		SearchReplace::MatchSet ms_stuff;
		ms_stuff.insert( s_stuff );
		ms_stuff.insert( r_stuff );
		sms.insert( &ms_stuff );

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

		SearchReplace::MatchSet ms_other_decls;
		ms_other_decls.insert( s_other_decls );
		ms_other_decls.insert( r_other_decls );
		sms.insert( &ms_other_decls );

		SearchReplace::MatchSet ms_other_statements;
		ms_other_statements.insert( s_comp->statements[0] );
		ms_other_statements.insert( r_comp->statements[1] );
		sms.insert( &ms_other_statements );

		SearchReplace::MatchSet ms_identifier;
		ms_identifier.insert( s_identifier );
		ms_identifier.insert( r_identifier->source );
		ms_identifier.insert( r_index_identifier->source );
		sms.insert( &ms_identifier );

		// Slightly hacky - since replace match sets are not yet two-pass
		// at the time of writing, we use the same node in all locations, and
		// therefore only that one node need appear in match set. This ensures
		// we can always key regardless of which location we see first while
		// walking the replace pattern.
		SearchReplace::MatchSet ms_new_identifier;
		ms_new_identifier.insert( r_identifier );
		sms.insert( &ms_new_identifier );
		SearchReplace::MatchSet ms_new_index_identifier;
		ms_new_index_identifier.insert( r_index_identifier );
		sms.insert( &ms_new_index_identifier );

/*		bool found = SearchReplace( s_fi, r_fi, sms ).SingleSearchReplace( program );
		if( !found )
			break;

		// Deal with appearances of the identifier
		ASSERT( ms_instance.key );
		shared_ptr<Instance> s_found_instance = dynamic_pointer_cast<Instance>(ms_instance.key->root);
		ASSERT( s_found_instance );
		ASSERT( ms_new_identifier.key );
		shared_ptr<Identifier> s_new_identifier = dynamic_pointer_cast<Identifier>(ms_new_identifier.key->root);
		ASSERT( s_new_identifier );
		ASSERT( ms_new_index_identifier.key );
		shared_ptr<Identifier> s_new_index_identifier = dynamic_pointer_cast<Identifier>(ms_new_index_identifier.key->root);
		ASSERT( s_new_index_identifier );

		shared_ptr<Subscript> r_sub( new Subscript );
		r_sub->base = s_new_identifier;
		r_sub->index = s_new_index_identifier;

		SearchReplace( s_found_instance->identifier, r_sub )( program );*/
	}
    // TODO insert dec before return statements
}
