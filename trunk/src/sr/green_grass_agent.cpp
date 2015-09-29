#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

void GreenGrassAgent::PatternQueryImpl() const
{
    RememberLink( false, AsAgent(GetThrough()) );
}


bool GreenGrassAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("G");
    
    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;
    
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( engine->GetOverallMaster()->dirty_grass.find( x ) != engine->GetOverallMaster()->dirty_grass.end() )
    {
        TRACE(*x)(" is dirty grass so rejecting\n");
        return false;
    }
    TRACE("subtree under ")(*x)(" is green grass\n");
    // Normal matching for the through path
    RememberLink( false, AsAgent(GetThrough()), x );
    return true;
}


TreePtr<Node> GreenGrassAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("G");
    ASSERT( GetThrough() );          
    TRACE("GreenGrass node through=")(*GetThrough())("\n");
    return AsAgent(GetThrough())->BuildReplace();
}


void GreenGrassAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The GreenGrass node appears as a small circle containing four vertical line characters,
	// like this: ||||. These are meant to represent the blades of grass. It was late and I was
	// tired.
	*bold = true;
	*shape = "circle";
	*text = string("||||");
}
