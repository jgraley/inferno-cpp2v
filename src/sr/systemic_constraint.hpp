#ifndef SYSTEMIC_CONSTRAINT
#define SYSTEMIC_CONSTRAINT

#include "constraint.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>

namespace SR
{
class Conjecture;    
}

namespace CSP
{ 

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
    explicit SystemicConstraint( SR::Agent *agent_ );
    
    // Documented in the base class
    int GetDegree() const;
    list<VariableId> GetVariables() const;
    bool Test( list< TreePtr<Node> > values, 
               SideInfo *side_info = nullptr );
        
private:
    class NormalLinkMismatch : public ::Mismatch
    {
    };

    SR::Agent * const agent;
    const shared_ptr<SR::PatternQuery> pq;
    const shared_ptr<SR::Conjecture> conj;
};

};

#endif