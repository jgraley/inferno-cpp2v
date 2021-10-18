#ifndef COMBINE_PATTERNS_HPP
#define COMBINE_PATTERNS_HPP

#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace SR 
{

class CombinePatterns : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( PatternKnowledge &pk ) override;
    void FixupPointers( PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp );
};    
        
}

#endif
