#ifndef COMBINE_PATTERNS_HPP
#define COMBINE_PATTERNS_HPP

#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace VN 
{

class CombinePatterns : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) override;
    void FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp );
};    
        
}

#endif
