#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


void GreenGrassAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                           XLink x ) const
{
    INDENT("G");
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.GetChildX().get());
    
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( master_scr_engine->GetOverallMaster()->dirty_grass.find( x.GetChildX() ) != master_scr_engine->GetOverallMaster()->dirty_grass.end() )
    {
        TRACE(x)(" is dirty grass so rejecting\n");
        throw Mismatch();            
    }
    TRACE("subtree under ")(x)(" is green grass\n");
    // Normal matching for the through path
    query.RegisterNormalLink( PatternLink(this, GetThrough()), x ); // Link into X
}


TreePtr<Node> GreenGrassAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("G");
    ASSERT( *GetThrough() );          
    TRACE("GreenGrass node through=")(**GetThrough())("\n");
    return AsAgent((TreePtr<Node>)*GetThrough())->BuildReplace();
}


Graphable::Block GreenGrassAgent::GetGraphBlockInfo() const
{
	// The GreenGrass node appears as a small circle containing four vertical line characters,
	// like this: ||||. These are meant to represent the blades of grass. It was late and I was
	// tired.
    Block block;
	block.bold = true;
	block.title = string("||||");
	block.shape = "circle";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "through", 
                           "", 
                           { { (TreePtr<Node>)*GetThrough(), 
                               SOLID, 
                               {},
                               {PatternLink(this, GetThrough()).GetShortName()} } } } };
    return block;
}
