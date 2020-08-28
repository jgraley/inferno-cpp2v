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

/** Implements a systemic constraint as discussed in #107
 */
class SystemicConstraint
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
    
    /**
     * Get the degree of the constraint.
     * 
     * @return The constraint's degree.
     */
    int GetDegree() const;
    
    /**
     * Test a list of variable values for inclusion in the constraint.
     * 
     * @param values the values of the variables, size should be the degree.
     * 
     * @retval true the values are in the constraint.
     * @retval false the values are not in the constraint
     */
    bool Test( std::list< TreePtr<Node> > values );
    
    //TODO need to add a public method for getting the variables so that
    // other constraints can be hooked up.
private:
    class Mismatch : public ::Mismatch
    {
    };

    Agent * const agent;
    const std::shared_ptr<PatternQuery> pq;
    const std::shared_ptr<Conjecture> conj;
};

};

#endif
