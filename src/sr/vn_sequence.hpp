#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"

class Graph;

/// SR namespace contains the search and replace implementation
namespace SR 
{

class VNTransformation;
class TheKnowledge;

/**
 * Vida Nova Sequence
 */
class VNSequence
{
public:    
    VNSequence( const vector< shared_ptr<VNTransformation> > &sequence );
    
    void PatternTransformations( int step_index );

    void PlanningStageOne( int step_index );
    void PlanningStageTwo( int step_index );
    void PlanningStageThree( int step_index );
    
    static void SetMaxReps( int step_index, int n, bool e );
    void SetStopAfter( int step_index, vector<int> ssa, int d=0 );    
        
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( int step_index,
                     TreePtr<Node> *proot );
    
    void ForSteps( function<void(int)> body );
    
    void DoGraph( int step_index, Graph &graph ) const;
    void GenerateGraphRegions( int step_index, Graph &graph ) const;
    string GetStepName( int step_index ) const;

    TheKnowledge *GetTheKnowledge();
    XLink UniquifyDomainExtension( XLink xlink ) const;
    XLink FindDomainExtension( XLink xlink ) const;
    void UpdateTheKnowledge( PatternLink root_plink, XLink root_xlink );

private:
    vector< shared_ptr<VNTransformation> > steps;
    shared_ptr<TheKnowledge> knowledge;    
};    
    
}

#endif
