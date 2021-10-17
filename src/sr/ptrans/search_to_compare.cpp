#include "search_to_compare.hpp"
#include "vn_transformation.hpp"
#include "agents/standard_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "agents/slave_agent.hpp"
   
using namespace SR;

void SearchToCompare::DoPatternTransformation( Info &info )
{    
    TreePtr<Node> scp = info.top_level_engine->GetSearchComparePattern();
	TreePtr<Node> rp = info.top_level_engine->GetReplacePattern();
    if( dynamic_pointer_cast<SearchReplace>(info.top_level_engine) )
        FixupPointers( info, scp, rp );
    info.vn_transformation->SetTopLevelEngine( make_shared<CompareReplace>() ); 
    info.top_level_engine = info.vn_transformation->GetTopLevelEngine(); // because it's a new one
    info.top_level_engine->Configure( scp, rp );
    
    for( SlaveAgent *sa : info.slaves )    
        if( sa->IsSearch() )
            FixupPointers( info, sa->search_pattern, sa->replace_pattern );         
}


void SearchToCompare::FixupPointers( Info &info, TreePtr<Node> &scp, TreePtr<Node> &rp )
{
    ASSERT( scp );
    ASSERT( scp==rp );
    //if( dynamic_pointer_cast<SearchReplace>(info.top_level_engine) ) // -> SearchToCompare
    {
        // Obtain search and replace semantics from a compare and replace engine
        // by inserting a stuff node at root
        MakePatternPtr< Stuff<Node> > stuff;        
        stuff->terminus = scp;
        scp = rp = stuff;   
    }
    ASSERT( scp );
    ASSERT( scp==rp );
}
