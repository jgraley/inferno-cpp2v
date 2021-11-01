#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"
#include "coupling.hpp"

using namespace SR;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


void GreenGrassAgent::RunColocatedQuery( XLink common_xlink ) const
{
    INDENT("G");
    
    // Restrict so that everything in the input program under here must be "green grass"
    // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
    if( master_scr_engine->GetOverallMaster()->dirty_grass.find( common_xlink.GetChildX() ) != 
        master_scr_engine->GetOverallMaster()->dirty_grass.end() ) // TODO .count() > 0
    {
        TRACE(*this)(" says ")(common_xlink)(" is dirty grass so rejecting\n");
        throw Mismatch();            
    }
    TRACE(*this)(" says ")(common_xlink)(" is green grass\n");
}


Graphable::Block GreenGrassAgent::GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const
{
	// The GreenGrass node appears as a triangle containing four vertical line characters,
	// like this: ||||. These are meant to represent the blades of grass. It was late and I was
	// tired.
    Block block;
	block.bold = true;
    block.title = "GreenGrass";
	block.symbol = "||||";
	block.shape = "cylinder";
    block.block_type = Graphable::NODE_SHAPED;
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>( GetThrough()->get()), 
              list<string>{},
              list<string>{PatternLink(this, GetThrough()).GetShortName()},
              phase,
              SpecialBase::IsNonTrivialPreRestriction(GetThrough()) );
    block.sub_blocks = { { "through", 
                           "", 
                           true,
                           { link } } };
    return block;
}
