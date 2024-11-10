#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/simple_duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    

Domain::Domain()
{
}
	
void Domain::PrepareDelete( DBWalk::Actions &actions )
{
	actions.push_back( [=](const DBWalk::WalkInfo &walk_info)
	{        
		EraseSolo( unordered_domain, walk_info.xlink );
	} );
}


void Domain::PrepareInsert(DBWalk::Actions &actions)
{
	actions.push_back( [=](const DBWalk::WalkInfo &walk_info)
	{        
		InsertSolo( unordered_domain, walk_info.xlink );   
	} );
}

