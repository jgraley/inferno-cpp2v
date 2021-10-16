#ifndef VN_TRANSFORMATION_HPP
#define VN_TRANSFORMATION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/transformation.hpp"

class Graph;

/// SR namespace contains the search and replace implementation
namespace SR 
{

class CompareReplace;
    
class VNTransformation : public InPlaceTransformation,
                         public virtual Graphable
{
public:    
    enum TransformationType
    {
        SEARCH_REPLACE,
        COMPARE_REPLACE
    };

    virtual void Configure( TransformationType type,
                            TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    
    void PatternTransformations();

    void PlanningStageOne();
    void PlanningStageTwo();
    void PlanningStageThree();
    
    static void SetMaxReps( int n, bool e );
    void SetStopAfter( vector<int> ssa, int d=0 );    
    
    using Transformation::operator();
    
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );
    
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
    virtual string GetGraphId() const; 
    void GenerateGraphRegions( Graph &graph ) const;

private:
    shared_ptr<CompareReplace> top_level_engine;
};    
    
}

#endif
