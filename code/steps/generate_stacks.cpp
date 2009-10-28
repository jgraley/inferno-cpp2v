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

GenerateStacks::GenerateStacks()
{
}


void GenerateStacks::operator()( shared_ptr<Program> program )
{
	SearchReplace sr0;

	shared_ptr<Instance> s_instance( new Instance );
	shared_ptr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
	s_instance->identifier = s_identifier;

	SearchReplace::MatchSet ms_identifier;
	ms_identifier.insert( s_identifier );

	set<SearchReplace::MatchSet *> s;
	s.insert( &ms_identifier );

	sr0.Configure( s_identifier, shared_ptr<Node>(), s );

	sr0( program );//.Compare( program, sr0.search_pattern, &(sr0.matches) );
	for( set<SearchReplace::MatchSet *>::iterator msi = sr0.matches.begin();
         msi != sr0.matches.end();
         msi++ )
    {
		TRACE("in S&R's match keys: %p %p %p\n", *msi, &((*msi)->key), (*msi)->key.get() );
	}
	TRACE("In our ms_identifier %p %p %p\n", &ms_identifier, &(ms_identifier.key), ms_identifier.key.get() );

	//ASSERT( ms_identifier.key );
}
