#include "slave_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

SlaveAgent::SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp ) :
    search_pattern( sp ),
    replace_pattern( rp )
{
}


bool SlaveAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj )
{
    // When a slave node seen duriung search, just forward through the "base" path
    return Agent::AsAgent(GetThrough())->DecidedCompare( x, can_key, conj );
}


void SlaveAgent::TrackingKey( Agent *from )
{
    // Make slaves "invisible" to Overlay key propagation
    DoKey( from->GetCoupled() );
    AsAgent(GetThrough())->TrackingKey(from);   
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT;
    ASSERT( GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = AsAgent(GetThrough())->BuildReplace();
    
    // Run the slave as a new transformation at the current location
    operator()( sr->GetContext(), &dest );
    
    ASSERT( dest );
    return dest;
}

