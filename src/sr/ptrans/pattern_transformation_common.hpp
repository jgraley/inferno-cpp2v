#ifndef PATTERN_TRANSFORMATION_COMMON_HPP
#define PATTERN_TRANSFORMATION_COMMON_HPP

#include "pattern_transformation.hpp"
#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include "search_replace.hpp"

namespace SR 
{

class VNTransformation;
class Agent;
class SlaveAgent;

class PatternTransformationCommon : public PatternTransformation
{
public:
    virtual void operator()( VNTransformation &vnt ) override;   

private:    
    void WalkPattern( set<PatternLink> &all_plinks, 
                      PatternLink plink ) const;
    
protected:
    struct PatternKnowledge
    {
        VNTransformation *vn_transformation;
        shared_ptr<CompareReplace> top_level_engine;
        TreePtr<Node> search_compare_root_pattern;
        Agent *search_compare_root_agent = nullptr;
        PatternLink search_compare_root_plink;
        TreePtr<Node> replace_root_pattern;
        Agent *replace_root_agent = nullptr;
        PatternLink replace_root_plink;
        set<PatternLink> all_plinks;
        set<PatternLink> slave_plinks;
    };
    
    virtual void DoPatternTransformation( PatternKnowledge &pk ) = 0;
};

}

#endif