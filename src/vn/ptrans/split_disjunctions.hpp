#ifndef SPLIT_DISJUNCTIONS_HPP
#define SPLIT_DISJUNCTIONS_HPP

#include "pattern_transformation_common.hpp"
#include "vn_step.hpp"
#include "search_replace.hpp"
#include "scr_engine.hpp"
#include "agents/agent.hpp"

namespace VN 
{
class DisjunctionAgent;

class SplitDisjunctions : public PatternTransformationCommon
{
    virtual void DoPatternTransformation( const PatternKnowledge &pk ) override;
    void MaybeSplit( DisjunctionAgent *da );
    void Split( DisjunctionAgent *da );
    TreePtr<Node> ReduceToNode( Collection<Node> &patterns, DisjunctionAgent *da );
};    
        
}

#endif
