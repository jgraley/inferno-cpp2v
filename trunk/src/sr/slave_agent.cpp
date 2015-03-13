#include "slave_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

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
    INDENT("l");
    // When a slave node seen duriung search, just forward through the "through" path
    RememberNormalLink( AsAgent(GetThrough()), x );
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


TreePtr<Node> SlaveAgent::BuildReplaceImpl( TreePtr<Node> keynode )
{
    INDENT("l");
    ASSERT( GetThrough() );   
    
    // Continue current replace operation by following the "through" pointer
    TreePtr<Node> dest = AsAgent(GetThrough())->BuildReplace();
    
    (void)Engine::RepeatingCompareReplace( &dest );   
    
    ASSERT( dest );
    return dest;
}
