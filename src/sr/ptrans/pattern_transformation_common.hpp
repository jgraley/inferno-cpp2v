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
protected:
    class PatternKnowledge
    {
    public:
        PatternKnowledge( VNTransformation &vnt );
    
    private:
        void WalkPattern( set<PatternLink> &all_plinks, 
                          PatternLink plink ) const;
                          
    public:
        VNTransformation *vn_transformation;
        shared_ptr<CompareReplace> top_level_engine;
        TreePtr<Node> search_compare_root_pattern;
        Agent *search_compare_root_agent = nullptr;
        PatternLink search_compare_root_plink;
        TreePtr<Node> replace_root_pattern;
        Agent *replace_root_agent = nullptr;
        PatternLink replace_root_plink;
        set<PatternLink> all_plinks;
        set<Agent *> all_agents;
        unordered_map< Agent *, unordered_set<PatternLink> > plinks_to_agents;                
        set<SlaveAgent *> slave_agents;
    };

public:
    virtual void operator()( VNTransformation &vnt ) override;   
    
protected:    
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) = 0;
};

}

#endif