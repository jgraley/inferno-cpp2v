#include "combine_patterns.hpp"
#include "vn_step.hpp"
#include "agents/standard_agent.hpp"
#include "agents/delta_agent.hpp"
#include "agents/embedded_scr_agent.hpp"

using namespace SR;

void CombinePatterns::DoPatternTransformation( const PatternKnowledge &pk )
{    
	(void)pk;
	// TODO use refs for scp and tp and in search to compare
    TreePtr<Node> scp = pk.search_compare_root_pattern;
    TreePtr<Node> rp = pk.replace_root_pattern;
    FixupPointers( pk, scp, rp );
    pk.root_engine->Configure( scp, rp );
        
    for( TreePtr<Node> node : pk.embedded_scr_nodes )    
    {
		EmbeddedSCRAgent *sa = dynamic_cast<EmbeddedSCRAgent *>(Agent::AsAgent(node));
		ASSERT( sa );
        FixupPointers( pk, sa->search_pattern, sa->replace_pattern );
    }
}


void CombinePatterns::FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
	(void)pk;
    ASSERT( scp );
    if( !rp )
    {
        rp = scp;
    }

    if( rp != scp ) // -> CombinePatterns
    {
        // Classic compare and replace with separate replace pattern, we can use
        // an Delta node to overwrite the replace pattern at replace time.
        auto delta = MakePatternNode< Delta<Node> >(); 
        delta->through = scp;
        delta->overlay = rp;
        scp = rp = delta; 
    }
    ASSERT( scp==rp );
}

