#ifndef VN_STEP_HPP
#define VN_STEP_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/transformation.hpp"
#include "ptrans/pattern_transformation.hpp"
#include "link.hpp"

namespace VN 
{
class Graph;
class Render;
class CompareReplace;
class VNSequence;
class XTreeDatabase;
class Lacing;

/**
 * Vida Nova Transformation    
 */
class VNStep : public virtual Graphable
{
public:    
    enum StepType
    {
        SEARCH_REPLACE, ///< Search pattern can match anywhere
        COMPARE_REPLACE ///< Compare pattern must match at root
    };

    virtual void Configure( StepType type,
                            TreePtr<Node> scp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    
    static PatternTransformationVector GetAllPatternTrans();
    void PatternTransformations();

    void PlanningStageOne( VNSequence *vn_sequence );
    void PlanningStageTwo();
    void PlanningStageThree();
    void PlanningStageFive( shared_ptr<const Lacing> lacing );
    
    static void SetMaxReps( int n, bool e );
    void SetStopAfter( vector<int> ssa, int d=0 );    
            
    void SetXTreeDb( shared_ptr<XTreeDatabase> x_tree_db );
    void Transform();
        
    NodeBlock GetGraphBlockInfo() const final;
    virtual string GetGraphId() const; 
    void GenerateGraphRegions( Graph &graph ) const;
    void DoRender( Render &render ) const;
    
    // For pattern transformations
    shared_ptr<CompareReplace> GetTopLevelEngine() const;
    void SetTopLevelEngine( shared_ptr<CompareReplace> tle );

private:
    shared_ptr<CompareReplace> root_engine;
    VNSequence *vn_sequence = nullptr;
    shared_ptr<XTreeDatabase> x_tree_db;
};    
    
}

#endif
