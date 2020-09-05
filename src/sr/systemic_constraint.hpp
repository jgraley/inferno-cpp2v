#ifndef SYSTEMIC_CONSTRAINT_HPP
#define SYSTEMIC_CONSTRAINT_HPP

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

class SimpleCompare;

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
     * @param agent the agent from which the constraint will be created.
     * 
     * @param coupling_residual_links the linked agents that should be tested using SimpleCompare
     */
    explicit SystemicConstraint( SR::Agent *agent, 
                                 const set<SR::Agent *> &coupling_residual_links );
    
private:
    int GetDegree() const;
    list<VariableId> GetVariables() const;
    bool Test( list< TreePtr<Node> > values, 
               SideInfo *side_info = nullptr );
        
    class NormalLinkMismatch : public ::Mismatch
    {
    };
    class CouplingResidualLinkMismatch : public ::Mismatch
    {
    };

    SR::Agent * const agent;
    const set<SR::Agent *> coupling_residual_links;
    const shared_ptr<SR::PatternQuery> pq;
    const shared_ptr<SR::Conjecture> conj;
    const shared_ptr<SimpleCompare> simple_compare;
};

};

#endif
