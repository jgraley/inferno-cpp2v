#ifndef PATTERN_TRANSFORMATION_HPP
#define PATTERN_TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

namespace SR 
{

class VNTransformation;
class Agent;
class SlaveAgent;

class PatternTransformation
{
public:
    virtual void operator()( VNTransformation &vnt ) = 0;   
};


class PatternTransformationVector : public vector< shared_ptr<PatternTransformation> >,
                                    public PatternTransformation
{
public:
    virtual void operator()( VNTransformation &vnt ) override;
};

}

#endif