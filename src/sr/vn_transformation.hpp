#ifndef VN_TRANSFORMATION_HPP
#define VN_TRANSFORMATION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/transformation.hpp"
#include "ptrans/pattern_transformation.hpp"

class Graph;

/// SR namespace contains the search and replace implementation
namespace SR 
{

class CompareReplace;
class VNSequence;

/**
 * Vida Nova Transformation    
 */
class VNTransformation : public InPlaceTransformation,
                         public virtual Graphable
{
public:    
    enum TransformationType
    {
        SEARCH_REPLACE, ///< Search pattern can match anywhere
        COMPARE_REPLACE ///< Compare pattern must match at root
    };

    virtual void Configure( TransformationType type,
                            TreePtr<Node> scp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    
    static PatternTransformationVector GetAllPatternTrans();
    void PatternTransformations();

    void PlanningStageOne( VNSequence *vn_sequence );
    void PlanningStageTwo();
    void PlanningStageThree();
    void PlanningStageFive();
    
    static void SetMaxReps( int n, bool e );
    void SetStopAfter( vector<int> ssa, int d=0 );    
    
    using Transformation::operator();
    
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );
    
    virtual Block GetGraphBlockInfo() const;
    virtual string GetGraphId() const; 
    void GenerateGraphRegions( Graph &graph ) const;
    
    // For pattern transformations
    shared_ptr<CompareReplace> GetTopLevelEngine() const;
    void SetTopLevelEngine( shared_ptr<CompareReplace> tle );

private:
    shared_ptr<CompareReplace> top_level_engine;
    VNSequence *vn_sequence = nullptr;
};    
    
}

#endif
