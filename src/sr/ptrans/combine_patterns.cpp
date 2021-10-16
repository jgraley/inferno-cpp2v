#include "combine_patterns.hpp"
#include "vn_transformation.hpp"
#include "agents/standard_agent.hpp"
#include "agents/overlay_agent.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;

void CombinePatterns::DoPatternTransformation( Info &info )
{    
    TreePtr<Node> scp = info.top_level_engine->GetSearchComparePattern();
	TreePtr<Node> rp = info.top_level_engine->GetReplacePattern();
    FixupPointers( info, scp, rp );
    info.top_level_engine->Configure( scp, rp );
    
    for( SlaveAgent *sa : info.slaves )
        FixupPointers( info, sa->search_pattern, sa->replace_pattern );
}


void CombinePatterns::FixupPointers( Info &info, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
    ASSERT( scp );
    if( !rp )
    {
        rp = scp;
    }

    if( rp != scp ) // -> CombinePatterns
    {
        // Classic compare and replace with separate replace pattern, we can use
        // an Overlay node to overwrite the replace pattern at replace time.
        MakePatternPtr< Overlay<Node> > overlay; 
        overlay->through = scp;
        overlay->overlay = rp;
        scp = rp = overlay; 
    }
    ASSERT( scp==rp );
}

