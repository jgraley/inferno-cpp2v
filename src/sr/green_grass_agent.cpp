#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" // TODO should not need

using namespace SR;

bool GreenGrassAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                          bool can_key,
                                          Conjecture &conj )
{
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( engine->GetOverallMaster()->dirty_grass.find( x ) != engine->GetOverallMaster()->dirty_grass.end() )
    {
        TRACE(*x)(" is dirty grass so rejecting\n");
        return false;
    }
    TRACE("subtree under ")(*x)(" is green grass\n");
    // Normal matching for the through path
    return Agent::AsAgent(GetThrough())->DecidedCompare( x, can_key, conj );
}


TreePtr<Node> GreenGrassAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERT( GetThrough() );          
    TRACE("GreenGrass node through=")(*GetThrough())("\n");
    return AsAgent(GetThrough())->BuildReplace();
}

