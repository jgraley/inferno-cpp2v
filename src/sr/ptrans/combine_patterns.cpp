#include "combine_patterns.hpp"
#include "vn_transformation.hpp"
#include "agents/standard_agent.hpp"
#include "agents/delta_agent.hpp"
#include "agents/slave_agent.hpp"

using namespace SR;

void CombinePatterns::DoPatternTransformation( const PatternKnowledge &pk )
{    
    TreePtr<Node> scp = pk.search_compare_root_pattern;
	TreePtr<Node> rp = pk.replace_root_pattern;
    FixupPointers( pk, scp, rp );
    pk.top_level_engine->Configure( scp, rp );
    
    for( PatternLink plink : pk.slave_plinks )
    {
        auto sa = dynamic_cast<SlaveAgent *>(plink.GetChildAgent());
        FixupPointers( pk, sa->search_pattern, sa->replace_pattern );
    }
}


void CombinePatterns::FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
    ASSERT( scp );
    if( !rp )
    {
        rp = scp;
    }

    if( rp != scp ) // -> CombinePatterns
    {
        // Classic compare and replace with separate replace pattern, we can use
        // an Delta node to overwrite the replace pattern at replace time.
        MakePatternPtr< Delta<Node> > overlay; 
        overlay->through = scp;
        overlay->overlay = rp;
        scp = rp = overlay; 
    }
    ASSERT( scp==rp );
}

