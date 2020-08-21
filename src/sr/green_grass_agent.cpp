#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( AsAgent(GetThrough()) );
    return pq;
}


void GreenGrassAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                    const TreePtrInterface *px ) const
{
    INDENT("G");
    ASSERT(px);
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(px->get());
    
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( engine->GetOverallMaster()->dirty_grass.find( *px ) != engine->GetOverallMaster()->dirty_grass.end() )
    {
        TRACE(**px)(" is dirty grass so rejecting\n");
        throw Mismatch();            
    }
    TRACE("subtree under ")(**px)(" is green grass\n");
    // Normal matching for the through path
    query.RegisterNormalLink( AsAgent(GetThrough()), px );
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
