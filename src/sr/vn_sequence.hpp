#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"

//#define NEW_KNOWLEDGE_UPDATE

class Graph;

/// SR namespace contains the search and replace implementation
namespace SR 
{

class VNStep;
class TheKnowledge;

/**
 * Vida Nova Sequence
 */
class VNSequence
{
public:    
    VNSequence( const vector< shared_ptr<VNStep> > &sequence );
    
    void PatternTransformations( int step_index );

    void PlanningStageOne( int step_index );
    void PlanningStageTwo( int step_index );
    void PlanningStageThree( int step_index );
    void PlanningStageFour();
    void PlanningStageFive( int step_index );
    
    static void SetMaxReps( int step_index, int n, bool e );
    void SetStopAfter( int step_index, vector<int> ssa, int d=0 );    
        
    void AnalysisStage( TreePtr<Node> root );
    TreePtr<Node> TransformStep( int step_index, TreePtr<Node> root );
    
    void ForSteps( function<void(int)> body );
    
    void DoGraph( int step_index, Graph &graph ) const;
    void GenerateGraphRegions( int step_index, Graph &graph ) const;
    string GetStepName( int step_index ) const;

    XLink UniquifyDomainExtension( XLink xlink ) const;
    XLink FindDomainExtension( XLink xlink ) const;
    void BuildTheKnowledge( XLink root_xlink );
    void ExtendDomain( PatternLink root_plink );

private:
    vector< shared_ptr<VNStep> > steps;
    shared_ptr<TheKnowledge> knowledge;    
    XLink initial_root_xlink;
};    
    
}

#endif
