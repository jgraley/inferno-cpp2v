#include "search_to_compare.hpp"
#include "vn_transformation.hpp"
#include "agents/standard_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "agents/slave_agent.hpp"
   
using namespace SR;

void SearchToCompare::DoPatternTransformation( PatternKnowledge &pk )
{    
    TreePtr<Node> scp = pk.search_compare_root_pattern;
	TreePtr<Node> rp = pk.replace_root_pattern;
    if( dynamic_pointer_cast<SearchReplace>(pk.top_level_engine) )
        FixupPointers( pk, scp, rp );
    pk.vn_transformation->SetTopLevelEngine( make_shared<CompareReplace>() ); 
    pk.top_level_engine = pk.vn_transformation->GetTopLevelEngine(); // because it's a new one
    pk.top_level_engine->Configure( scp, rp );
    
    for( PatternLink plink : pk.slave_plinks )    
    {
        auto sa = dynamic_cast<SlaveAgent *>(plink.GetChildAgent());
        if( sa->IsSearch() )
        {
            scp = sa->search_pattern;
            rp = sa->replace_pattern;
            FixupPointers( pk, scp, rp );

            //TreePtr<Node> t = sa->GetThrough();
            //MakePatternPtr< SlaveCompareReplace<Scope> > sa_compare( t, scp, rp );
            //(sa_compare) sa_compare;
            
            sa->search_pattern = scp;
            sa->replace_pattern = rp;
        }
    }
}


void SearchToCompare::FixupPointers( PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
    ASSERT( scp );
    ASSERT( scp==rp );

    // Obtain search and replace semantics from a compare and replace engine
    // by inserting a stuff node at root
    MakePatternPtr< Stuff<Node> > stuff;        
    stuff->terminus = scp;
    scp = rp = stuff;   
    
    ASSERT( scp );
    ASSERT( scp==rp );
}
