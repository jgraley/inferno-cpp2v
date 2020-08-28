#ifndef SYSTEMIC_CONSTRAINT
#define SYSTEMIC_CONSTRAINT

#include "node/node.hpp"
#include "common/common.hpp"
#include <memory>
#include <list>
#include <memory>


namespace SR
{ 
    
class Agent;
class PatternQuery;
class DecidedQuery;
class Conjecture;

class SystemicConstraint
{
public:
    class Mismatch : public ::Mismatch
    {
    };

    explicit SystemicConstraint( Agent *agent_ );
    
    int GetDegree() const;
    bool Test( std::list< TreePtr<Node> > values );
    
private:
    Agent * const agent;
    const std::shared_ptr<PatternQuery> pq;
    const std::shared_ptr<Conjecture> conj;
};

};

#endif
