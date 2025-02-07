#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/simple_duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    

Domain::Domain()
{
}


DBWalk::Action Domain::GetDeleteAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{        
		EraseSolo( unordered_domain, walk_info.xlink );
	};
}


DBWalk::Action Domain::GetInsertAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{        
		InsertSolo( unordered_domain, walk_info.xlink );   
	};
}

