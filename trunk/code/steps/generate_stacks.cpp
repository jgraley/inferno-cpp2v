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
	set<SearchReplace::MatchSet *> sms;

	shared_ptr<Instance> s_instance( new Instance );
	shared_ptr<InstanceIdentifier> s_identifier( new InstanceIdentifier );
	s_instance->identifier = s_identifier;

	SearchReplace::MatchSet ms_identifier;
	ms_identifier.insert( s_identifier );
	sms.insert( &ms_identifier );

	SearchReplace( s_identifier, shared_ptr<Node>(), sms )( program );

	ASSERT( ms_identifier.key );
}
