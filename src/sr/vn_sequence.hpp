#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"
#include "sr/db/lacing.hpp"
#include "sr/db/domain_extension.hpp"

class Graph;


namespace SR 
{

class VNStep;
class XTreeDatabase;
class Command;

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

    void CompleteDomainExtension();
    void ExecuteUpdateCommand( Command *cmd,
                               stack<FreeZone> *free_zone_stack );
    
    void XTreeDbDump() const;
    void XTreeDbExpectMatches() const;
    
    bool IsDirtyGrass( TreePtr<Node> node ) const;
    void AddDirtyGrass( TreePtr<Node> node ) const;
    
private:
    vector< shared_ptr<VNStep> > steps;
    shared_ptr<Lacing> lacing;
    DomainExtension::ExtenderSet domain_extenders;
    shared_ptr<XTreeDatabase> x_tree_db;    
};    
    
}

#endif
