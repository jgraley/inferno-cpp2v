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
    void WalkPattern( set<Agent *> &children, 
                      Agent *agent ) const;
    
protected:
    struct Info
    {
        VNTransformation *vn_transformation;
        shared_ptr<CompareReplace> top_level_engine;
        set<Agent *> children;
        set<SlaveAgent *> slaves;
    };
    
    virtual void DoPatternTransformation( Info &info ) = 0;
};

}

#endif