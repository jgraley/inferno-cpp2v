#include "search_to_compare.hpp"
#include "vn_step.hpp"
#include "agents/standard_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "agents/embedded_scr_agent.hpp"
   
using namespace SR;

void SearchToCompare::DoPatternTransformation( const PatternKnowledge &pk )
{    
    TreePtr<Node> scp = pk.search_compare_root_pattern;
	TreePtr<Node> rp = pk.replace_root_pattern;
    if( dynamic_pointer_cast<SearchReplace>(pk.top_level_engine) )
        FixupPointers( pk, scp, rp );
    auto new_tle = make_shared<CompareReplace>(); 
    new_tle->Configure( scp, rp );
    pk.vn_transformation->SetTopLevelEngine(new_tle); // install the new one
    
    for( EmbeddedSCRAgent *sa : pk.slave_agents )    
    {
        if( sa->IsSearch() )
        {                    
            FixupPointers( pk, sa->search_pattern, sa->replace_pattern );

            TreePtr<Node> nn = sa->EvolveIntoEmbeddedCompareReplace();            
            for( PatternLink plink : pk.plinks_to_agents.at(sa) )
                plink.Redirect(nn);           
        }
    }
}


void SearchToCompare::FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
    ASSERT( scp );
    ASSERT( scp==rp );

    // Obtain search and replace semantics from a compare and replace engine
    // by inserting a stuff node at root
    auto stuff = MakePatternNode< Stuff<Node> >();        
    stuff->terminus = scp;
    scp = rp = stuff;   
    
    ASSERT( scp );
    ASSERT( scp==rp );
}
