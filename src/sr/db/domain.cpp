#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/simple_duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    

Domain::Domain()
{
}


void Domain::DeleteGeometricAction(const DBWalk::WalkInfo &walk_info)
{        
    EraseSolo( unordered_domain, walk_info.xlink );
    //TRACE("DELETE ")(walk_info.xlink)(" is_terminus=")(walk_info.at_terminus)("\n");
}


void Domain::InsertGeometricAction(const DBWalk::WalkInfo &walk_info)
{
    InsertSolo( unordered_domain, walk_info.xlink );   
    //TRACE("INSERT ")(walk_info.xlink)(" is_terminus=")(walk_info.at_terminus)("\n");
}


void Domain::CheckEqual( shared_ptr<Domain> l, shared_ptr<Domain> r )
{
    ASSERT( l->unordered_domain == r->unordered_domain )(" domain mismatch:\n")(DiffTrace(l->unordered_domain, r->unordered_domain));
}
