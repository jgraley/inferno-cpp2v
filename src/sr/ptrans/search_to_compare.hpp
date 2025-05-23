#ifndef SEARCH_TO_COMPARE_HPP
#define SEARCH_TO_COMPARE_HPP

#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace SR 
{

class SearchToCompare : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) override;
    void FixupPointers( const PatternKnowledge &pk, TreePtr<Node> &scp, TreePtr<Node> &rp );
};    
        
}

#endif
