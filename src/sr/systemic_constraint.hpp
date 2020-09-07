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
     * @param vql callback that requests information about variables
     */
    explicit SystemicConstraint( SR::Agent *agent, 
                                 VariableQueryLambda vql );
    
private:
    int GetDegree() const;
    list<VariableId> GetVariables() const { return GetVariablesImpl(agent, pq); }
    static list<VariableId> GetVariablesImpl( SR::Agent * const agent, 
                                              shared_ptr<SR::PatternQuery> pq );
    static list<VariableFlags> GetFlags( list<VariableId> vars, VariableQueryLambda vql );
    bool Test( list< TreePtr<Node> > values, 
               SideInfo *side_info = nullptr );
        
    class ByLocationLinkMismatch : public ::Mismatch
    {
    };
    class ByValueLinkMismatch : public ::Mismatch
    {
    };

    SR::Agent * const agent;
    const shared_ptr<SR::PatternQuery> pq;
    const list<VariableFlags> flags;
    
    const shared_ptr<SR::Conjecture> conj;
    const shared_ptr<SimpleCompare> simple_compare;
};

};

#endif
