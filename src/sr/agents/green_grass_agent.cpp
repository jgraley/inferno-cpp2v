#include "green_grass_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( GetThrough() );
    return pq;
}


void GreenGrassAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                           TreePtr<Node> x ) const
{
    INDENT("G");
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.get());
    
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( master_scr_engine->GetOverallMaster()->dirty_grass.find( x ) != master_scr_engine->GetOverallMaster()->dirty_grass.end() )
    {
        TRACE(*x)(" is dirty grass so rejecting\n");
        throw Mismatch();            
    }
    TRACE("subtree under ")(*x)(" is green grass\n");
    // Normal matching for the through path
    query.RegisterNormalLink( GetThrough(), x ); // Link into X
}


TreePtr<Node> GreenGrassAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("G");
    ASSERT( *GetThrough() );          
    TRACE("GreenGrass node through=")(**GetThrough())("\n");
    return AsAgent(*GetThrough())->BuildReplace();
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
