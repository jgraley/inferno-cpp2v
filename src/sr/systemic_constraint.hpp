#ifndef SYSTEMIC_CONSTRAINT
#define SYSTEMIC_CONSTRAINT

#include "node/node.hpp"
#include "common/common.hpp"

#include "query.hpp"

#include <memory>
#include <list>
#include <memory>


namespace SR
{ 
    
class Agent;

class Conjecture;

/** Implements a systemic constraint as discussed in #107
 */
class SystemicConstraint
{
public:
    struct SideInfo
    {
        Set< shared_ptr<const DecidedQuery> > evaluator_queries;   
        Set< std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > > abnormal_links; 
        Set< std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > > multiplicity_links;
    };

    /**
     * Embed the assumption that variables are simply 1:1 with agents. Note
     * that this class will _always_ take and Agent * in its constructor, 
     * because that's what "systemic constraint" means - and so the constructor
     * takes Agent * explicitly. But the variables could change, so we 
     * introduce a typedef for them.
     */
    typedef Agent * VariableId;

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
     * Get the degree of the constraint.
     * 
     * @return A list of variables affecteed by this constraint. Size equals the return from GetDegree()
     */
    std::list<VariableId> GetVariables() const;
    
    /**
     * Test a list of variable values for inclusion in the constraint.
     * 
     * @param values [in] the values of the variables, size should be the degree.
     * @param side_info [out] information relating to abnormal contexts etc, only defined if true is returned.
     * 
     * @retval true the values are in the constraint, same ordering as return of GetVariables().
     * @retval false the values are not in the constraint
     */
    bool Test( std::list< TreePtr<Node> > values, 
               SideInfo *side_info = nullptr );
        
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
