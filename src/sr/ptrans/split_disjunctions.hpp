#ifndef SPLIT_DISJUNCTIONS_HPP
#define SPLIT_DISJUNCTIONS_HPP

#include "pattern_transformation_common.hpp"
#include "vn_transformation.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace SR 
{
class DisjunctionAgent;

class SplitDisjunctions : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) override;
    void MaybeSplit( DisjunctionAgent *da );
    void Split( DisjunctionAgent *da );
};    
        
}

#endif
