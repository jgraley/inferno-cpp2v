#include "slave_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

SlaveAgent::SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
    Engine( is_search ),
    search_pattern( sp ),
    replace_pattern( rp ),
    master_coupling_keys( NULL )
{
}


bool SlaveAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("l");

    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    // When a slave node seen duriung search, just forward through the "through" path
    RememberLink( false, AsAgent(GetThrough()), x );
    return true;
}


void SlaveAgent::TrackingKey( Agent *from )
{
    INDENT("l");
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


void SlaveAgent::Configure( const Set<Agent *> &agents_already_configured, const Engine *master )
{
    ASSERT(master); // must not be overall master (i.e. NULL)
    Engine::Configure(search_pattern, replace_pattern, agents_already_configured, master);
}    


void SlaveAgent::SetMasterCouplingKeys( CouplingMap &keys )
{
	master_coupling_keys = &keys;
}


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode )
{
    INDENT("l");
    ASSERT( GetThrough() );   
    ASSERT( master_coupling_keys );
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = AsAgent(GetThrough())->BuildReplace();
    
    // Tell master engine to fill in any couplings generated by replace-only 
    // nodes resulting from following the "through" pointer
    engine->GatherCouplings( *master_coupling_keys );
    
    // Run the slave engine
    (void)Engine::RepeatingCompareReplace( &dest, *master_coupling_keys );   
    
    ASSERT( dest );
    return dest;
}