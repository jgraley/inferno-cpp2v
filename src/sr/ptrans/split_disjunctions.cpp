#include "split_disjunctions.hpp"
#include "vn_transformation.hpp"
#include "agents/disjunction_agent.hpp"
   
using namespace SR;

void SplitDisjunctions::DoPatternTransformation( const PatternKnowledge &pk )
{    
    for( Agent *agent : pk.all_agents )
        if( auto da = dynamic_cast<DisjunctionAgent *>(agent) )
            MaybeSplit(da);            
}


void SplitDisjunctions::MaybeSplit( DisjunctionAgent *da )
{
    CollectionInterface &patterns = da->GetPatterns();
    switch( patterns.size() )
    {
        case 0:
        case 1:
            ASSERT(false)(*da)(" must have at least 2 children but got ")(patterns.size());
        case 2:
            break; // nothing to do
        default:
            Split( da );
            break;
    }
}

void SplitDisjunctions::Split( DisjunctionAgent *da )
{
    CollectionInterface &patterns = da->GetPatterns();
    int size = patterns.size();
    //FTRACE("Would split ")(*da)(" with %d children", patterns.size())("\n");
    int split_point = (size+1)/2; // round up
    
    Collection<Node> l_patterns;
    Collection<Node> r_patterns;
    int i;
    CollectionInterface::iterator pit;
    for( pit = patterns.begin(), i=0; 
         pit != patterns.end(); 
         ++pit, ++i )    
    {
        if( i < split_point )
            l_patterns.insert( *pit );
        else
            r_patterns.insert( *pit );            
    }
    
    Collection<Node> new_patterns;    
    
    ASSERT( !l_patterns.empty() );
    if( l_patterns.size() >= 2 )
    {
        TreePtr<Node> l_dis = da->CloneToEmpty();
        new_patterns.insert( l_dis );
        
        auto lda = dynamic_cast<DisjunctionAgent *>( Agent::AsAgent( l_dis ) );
        lda->SetPatterns( l_patterns );
        MaybeSplit(lda);
    }
    else
    {
        new_patterns.insert( *(l_patterns.begin()) );
    }

    ASSERT( !r_patterns.empty() );
    if( r_patterns.size() >= 2 )
    {
        TreePtr<Node> r_dis = da->CloneToEmpty();    
        new_patterns.insert( r_dis );
        
        auto rda = dynamic_cast<DisjunctionAgent *>( Agent::AsAgent( r_dis ) );
        rda->SetPatterns( r_patterns );  
        MaybeSplit(rda); 
    }
    else
    {
        new_patterns.insert( *(r_patterns.begin()) );
    }
    
    da->SetPatterns( new_patterns );
}
