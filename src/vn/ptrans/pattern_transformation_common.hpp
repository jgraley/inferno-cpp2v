#ifndef PATTERN_TRANSFORMATION_COMMON_HPP
#define PATTERN_TRANSFORMATION_COMMON_HPP

#include "pattern_transformation.hpp"
#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include "search_replace.hpp"

namespace VN 
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
        shared_ptr<CompareReplace> root_engine;
        
		// Pattern link memory safety: declare these before any plinks that hang off them
        TreePtr<Node> search_compare_root_pattern;
        TreePtr<Node> replace_root_pattern;

        Agent *search_compare_root_agent = nullptr;
        Agent *replace_root_agent = nullptr;

        PatternLink search_compare_root_plink;
        PatternLink replace_root_plink;

        set<TreePtr<Node>> all_pattern_nodes;
        set<TreePtr<Node>> embedded_scr_nodes;

		// Pattern link memory safety: declare plinks AFTER TreePtrs that hold things in place
        map< Agent *, set<PatternLink> > agents_to_incoming_plinks;                
    };

public:
    virtual void operator()( VNStep &vnt ) override;   
    
protected:    
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) = 0;
};

}

#endif
