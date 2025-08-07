#ifndef PATTERN_TRANSFORMATION_COMMON_HPP
#define PATTERN_TRANSFORMATION_COMMON_HPP

#include "pattern_transformation.hpp"
#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include "search_replace.hpp"

namespace SR 
{

class VNStep;
class Agent;
class EmbeddedSCRAgent;

class PatternTransformationCommon : public PatternTransformation
{
protected:
    class PatternKnowledge
    {
    public:
        PatternKnowledge( VNStep &vnt, const PatternTransformationCommon *trans );
    
    private:
        void WalkPattern( set<PatternLink> &all_plinks, 
                          PatternLink plink ) const;
                          
    public:
        VNStep *vn_transformation;
        shared_ptr<CompareReplace> top_level_engine;
        TreePtr<Node> search_compare_root_pattern;
        Agent *search_compare_root_agent = nullptr;
        PatternLink search_compare_root_plink;
        TreePtr<Node> replace_root_pattern;
        Agent *replace_root_agent = nullptr;
        PatternLink replace_root_plink;
        set<Agent *> all_agents;
        map< Agent *, set<PatternLink> > agents_to_incoming_plinks;                
        set<EmbeddedSCRAgent *> embedded_scr_agents;
    };

public:
    virtual void operator()( VNStep &vnt ) override;   
    
protected:    
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) = 0;
};

}

#endif
