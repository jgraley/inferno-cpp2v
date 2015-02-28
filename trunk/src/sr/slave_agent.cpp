#include "slave_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

SlaveAgent::SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
    Engine( is_search ),
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


void SlaveAgent::GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const
{
    labels->push_back("through");
    links->push_back(GetThrough());
    Engine::GetGraphInfo( labels, links );
}


void SlaveAgent::Configure( const Set<Agent *> &agents_already_configured )
{
    Engine::Configure(search_pattern, replace_pattern, agents_already_configured);
}    


void SlaveAgent::AgentConfigure( const Engine *e )
{
    AgentCommon::AgentConfigure( e );
    Engine::master_ptr = e;
}       


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode )
{
    INDENT;
    ASSERT( GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = AsAgent(GetThrough())->BuildReplace();
    
    (void)Engine::RepeatingCompareReplace( &dest );   
    
    ASSERT( dest );
    return dest;
}
