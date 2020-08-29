#ifndef SYSTEMIC_CONSTRAINT
#define SYSTEMIC_CONSTRAINT

#include "constraint.hpp"

#include "query.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>


namespace SR
{ 
    
class Agent;

class Conjecture;

/** Implements a systemic constraint as discussed in #107
 */
class SystemicConstraint : public Constraint
{
public:
    /**
     * Create the constraint. 
     * 
     * The constraint is permanently associated with the supplied agent.
     * 
     * @param agnet_ the agent from which the constraint will be created.
     */
    explicit SystemicConstraint( Agent *agent_ );
    
    // Documented in the base class
    int GetDegree() const;
    std::list<VariableId> GetVariables() const;
    bool Test( std::list< TreePtr<Node> > values, 
               SideInfo *side_info = nullptr );
        
private:
    class NormalLinkMismatch : public ::Mismatch
    {
    };

    Agent * const agent;
    const std::shared_ptr<PatternQuery> pq;
    const std::shared_ptr<Conjecture> conj;
};

};

#endif
