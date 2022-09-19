#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"

class Graph;


namespace SR 
{

class VNStep;
class XTreeDatabase;
class UpdateCommand;

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
    void ExtendDomainNewPattern( PatternLink root_plink );

    void ExecuteUpdateCommand( shared_ptr<UpdateCommand> cmd );
    void XTreeDbBuildNonIncremental();
    
    bool IsDirtyGrass( TreePtr<Node> node );
    void AddDirtyGrass( TreePtr<Node> node );
    
private:
    vector< shared_ptr<VNStep> > steps;
    shared_ptr<XTreeDatabase> x_tree_db;    
    set< TreePtr<Node> > dirty_grass;
    XLink current_root_xlink;
};    
    
}

#endif
