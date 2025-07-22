#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/simple_duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    

Domain::Domain()
{
}


void Domain::InsertTree(TreeZone &zone)
{     
	db_walker.WalkTreeZone( bind(&Domain::InsertAction, this, placeholders::_1), 
	                        zone, DBWalk::WIND_IN );
}


void Domain::DeleteTree(TreeZone &zone)
{
	db_walker.WalkTreeZone( bind(&Domain::DeleteAction, this, placeholders::_1), 
	                        zone, DBWalk::WIND_OUT );
}


void Domain::InsertAction(const DBWalk::WalkInfo &walk_info)
{
    //TRACE("INSERT ")(walk_info.xlink)(" is_terminus=")(walk_info.at_terminus)("\n");
    InsertSolo( unordered_domain, walk_info.xlink );   
}


void Domain::DeleteAction(const DBWalk::WalkInfo &walk_info)
{        
    //TRACE("DELETE ")(walk_info.xlink)(" is_terminus=")(walk_info.at_terminus)("\n");
    EraseSolo( unordered_domain, walk_info.xlink );
}


void Domain::CheckEqual( shared_ptr<Domain> l, shared_ptr<Domain> r )
{
    ASSERT( l->unordered_domain == r->unordered_domain )(" domain mismatch:\n")(DiffTrace(l->unordered_domain, r->unordered_domain));
}


size_t Domain::GetTotNumXLinks() const
{
	return unordered_domain.size();
}
