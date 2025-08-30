#include "search_to_compare.hpp"
#include "vn_step.hpp"
#include "agents/standard_agent.hpp"
#include "agents/depth_agent.hpp"
#include "agents/embedded_scr_agent.hpp"
   
using namespace SR;

void SearchToCompare::DoPatternTransformation( const PatternKnowledge &pk )
{    
    TreePtr<Node> scp = pk.search_compare_root_pattern;
    TreePtr<Node> rp = pk.replace_root_pattern;
    if( dynamic_pointer_cast<SearchReplace>(pk.root_engine) )
        FixupPointers( pk, scp, rp );
    auto new_tle = make_shared<CompareReplace>(); 
    new_tle->Configure( scp, rp );
    pk.vn_transformation->SetTopLevelEngine(new_tle); // install the new one
    
    for( TreePtr<Node> node : pk.embedded_scr_nodes )    
    {
		EmbeddedSCRAgent *sa = dynamic_cast<EmbeddedSCRAgent *>(Agent::AsAgent(node));
		ASSERT( sa );
        if( sa->IsSearch() )
        {                    
            FixupPointers( pk, sa->search_pattern, sa->replace_pattern );
            			
            TreePtr<Node> nn = sa->EvolveIntoEmbeddedCompareReplace();            
            for( PatternLink plink : pk.agents_to_incoming_plinks.at(sa) )
                plink.Redirect(nn);           
        }
    }
}

bool SearchToCompare::RequireAgentsToIncomingPlinksMap() const
{
	return true; // yes please and we promise to keep them valid or delete them
}


void SearchToCompare::FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
	(void)pk;
    ASSERT( scp );
	ASSERT( scp==rp ); 	// Should have filled in both by now (CombinePatterns)
	
    // Obtain search and replace semantics from a compare and replace engine
    // by inserting a stuff node at root
    auto stuff = MakePatternNode< Stuff<Node> >();        
    stuff->terminus = scp;
    scp = rp = stuff;   
    
    ASSERT( scp );
    ASSERT( scp==rp );
}
