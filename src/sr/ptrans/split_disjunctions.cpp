#include "split_disjunctions.hpp"
#include "vn_step.hpp"
#include "agents/disjunction_agent.hpp"
   
using namespace SR;

void SplitDisjunctions::DoPatternTransformation( const PatternKnowledge &pk )
{    
    for( TreePtr<Node> node : pk.all_pattern_nodes )
        if( auto da = dynamic_cast<DisjunctionAgent *>(Agent::AsAgent(node)) )
            MaybeSplit(da);            
}


void SplitDisjunctions::MaybeSplit( DisjunctionAgent *da )
{
    CollectionInterface &patterns = da->GetDisjuncts();
    switch( patterns.size() )
    {
        case 0:
        case 1:
            ASSERT(false)(*da)(" must have at least 2 children but got ")(patterns.size());
        case 2:
            break; // nothing to do
        default: // >= 3
            Split( da );
            break;
    }
}

void SplitDisjunctions::Split( DisjunctionAgent *da )
{
    CollectionInterface &patterns = da->GetDisjuncts();
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
    new_patterns.insert( ReduceToNode(l_patterns, da) );
    new_patterns.insert( ReduceToNode(r_patterns, da) );    
    da->SetDisjuncts( new_patterns );
}


TreePtr<Node> SplitDisjunctions::ReduceToNode( Collection<Node> &patterns, DisjunctionAgent *da )
{
    switch( patterns.size() )
    {
    case 0:
        ASSERTFAIL();        
    case 1:
        return *(patterns.begin()); // return the only element, no new node required
    default: // >=2
        {
            TreePtr<Node> new_dnode = da->CloneToEmpty();
            
            auto new_da = dynamic_cast<DisjunctionAgent *>( Agent::AsAgent( new_dnode ) );
            new_da->SetDisjuncts( patterns );
            MaybeSplit(new_da);

            return new_dnode;
        }
    }    
}

