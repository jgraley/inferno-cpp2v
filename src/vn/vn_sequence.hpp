#ifndef VN_SEQUENCE_HPP
#define VN_SEQUENCE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"
#include "db/lacing.hpp"
#include "db/domain_extension.hpp"
#include "up/up_common.hpp"
#include "lang/render.hpp"

class Graph;


namespace VN 
{

class VNStep;
class XTreeDatabase;
class TreeUpdater;
/**
 * Vida Nova Sequence
 */
class VNSequence
{
public:    
    explicit VNSequence( const vector< shared_ptr<VNStep> > &sequence );
    ~VNSequence();
    
    void PatternTransformations( int step_index );

    void PlanningStageOne( int step_index );
    void PlanningStageTwo( int step_index );
    void PlanningStageThree( int step_index );
    void PlanningStageFour();
    void PlanningStageFive( int step_index );
    
    static void SetMaxReps( int n, bool e );
    void SetStopAfter( int step_index, vector<int> ssa, int d=0 );    
        
    void AnalysisStage( TreePtr<Node> main_tree_root );
    TreePtr<Node> TransformStep( int step_index );
    
    void ForSteps( function<void(int)> body );
    
    void DoGraph( int step_index, Graph &graph ) const;
    void GenerateGraphRegions( int step_index, Graph &graph ) const;
    string GetStepName( int step_index ) const;
    void DoRender( int step_index, Render &render ) const;

    TreeUpdater *GetTreeUpdater() const;
    
    void XTreeDbDump() const;
    void XTreeDbExpectMatches() const;
    
private:
    vector< shared_ptr<VNStep> > steps;
    shared_ptr<Lacing> lacing;
    DomainExtension::ExtenderSet domain_extenders;
    shared_ptr<XTreeDatabase> x_tree_db;  
    unique_ptr<TreeUpdater> tree_updater;
};    
    
}

#endif
