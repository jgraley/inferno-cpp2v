#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need
#include "star_agent.hpp" // TODO should not need

using namespace SR;

bool OverlayAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                          bool can_key,
                                          Conjecture &conj )
{
    return Agent::AsAgent(GetThrough())->DecidedCompare( x, can_key, conj );
}


void OverlayAgent::KeyReplace()
{
    INDENT("O");
    // Give the overlay side a nudge in case it wants to key itself eg due to
    // Builder node. TODO avoid repeat calls to KeyReplace()    
    Agent::AsAgent(GetOverlay())->KeyReplace();
    
    // Now, if the overlay side did not key itself, key it per the through 
    // side key that will have been obtained during search. Thus, the immediate
    // children of Overlay appear as though coupled.
    if( !coupling_keys->GetCoupled( Agent::AsAgent(GetOverlay()) ) )
    {
        // Get a key from the search side
        TreePtr<Node> keynode = coupling_keys->GetCoupled( Agent::AsAgent(GetThrough()) );
        ASSERT(keynode);
        // Key as many nodes as possible on the replace side
        Agent *over = Agent::AsAgent(GetOverlay());
        over->SetReplaceKey(keynode);
    }
}


TreePtr<Node> OverlayAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERT( GetOverlay() );          
    TRACE("Overlay node through=")(*(GetThrough()))(" overlay=")(*(GetOverlay()))("\n");
    return AsAgent(GetOverlay())->BuildReplace();
}


bool InsertAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                      bool can_key,
                                      Conjecture &conj )
{
    ASSERTFAIL(*this)(" found outside of a container; can only be used in containers\n");
}


TreePtr<Node> InsertAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERTFAIL(*this)(" found outside of a container; can only be used in containers\n");
}


bool EraseAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj )
{
    ASSERTFAIL(*this)(" found outside of a container; can only be used in containers\n");    
}


TreePtr<Node> EraseAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERTFAIL(*this)(" found outside of a container; can only be used in containers\n");
}

Sequence<Node> OverlayAgent::WalkContainerPattern( ContainerInterface &pattern,
                                                   bool replacing ) 
{
    // This helper is for Insert and Erase nodes. It takes a pattern container (which
    // is the only place these nodes should occur) and expands out either Insert or
    // Erase nodes. When searching, Erase is expanded out so that the program nodes
    // to be erased may be matched off (cond keyed etc) and Insert is skipped because
    // it does not need to correspond to anything during search. When replacing, 
    // erase is skipped to erase the elements and Insert is expanded to insert them. 
    Sequence<Node> expanded;
    FOREACH( TreePtr<Node> n, pattern )
    {
        if( shared_ptr<EraseAgent> pe = dynamic_pointer_cast<EraseAgent>(n) )
        {
            if( !replacing )
                FOREACH( TreePtr<Node> e, *(pe->GetErase()) )
                    expanded.push_back( e );                
        }
        else if( shared_ptr<InsertAgent> pi = dynamic_pointer_cast<InsertAgent>(n) )
        {
            if( replacing )
                FOREACH( TreePtr<Node> i, *(pi->GetInsert()) )
                    expanded.push_back( i );                
        }
        else
        {
            expanded.push_back( n );
        }
    }
    return expanded;
}
