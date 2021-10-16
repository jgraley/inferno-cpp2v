#ifndef SEARCH_TO_COMPARE_HPP
#define SEARCH_TO_COMPARE_HPP

#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace SR 
{

class SearchToCompare : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( Info &info ) override;
    void FixupPointers( Info &info, TreePtr<Node> &scp, TreePtr<Node> &rp );
};    
        
}

#endif
