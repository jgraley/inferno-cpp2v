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
    
class VNTransformation : public InPlaceTransformation
{
public:    
    enum Type
    {
        SEARCH_REPLACE,
        COMPARE_REPLACE
    };

    virtual void Configure( Type type,
                            TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    
    void PatternTransformations();

    void PlanningStageOne();
    void PlanningStageTwo();
    void PlanningStageThree();
    
    using Transformation::operator();
    
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );
    
private:
    shared_ptr<CompareReplace> top_level_engine;
};    
    
}

#endif
