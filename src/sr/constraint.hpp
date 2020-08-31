#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include "query.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>

namespace SR
{
class Agent;
class BooleanEvaluator;
}

namespace CSP
{ 
struct SideInfo
{
    Set< pair< shared_ptr<SR::BooleanEvaluator>, SR::DecidedQuery::Links > > evaluator_records;   
    Set< shared_ptr<const SR::DecidedQuery::Link> > abnormal_links; 
    Set< shared_ptr<const SR::DecidedQuery::Link> > multiplicity_links;
};

/**
 * Embed the assumption that variables are simply 1:1 with agents. Note
 * that this class will _always_ take and Agent * in its constructor, 
 * because that's what "systemic constraint" means - and so the constructor
 * takes Agent * explicitly. But the variables could change, so we 
 * introduce a typedef for them.
 */
typedef SR::Agent * VariableId;

enum ValueCategory
{
    BY_LOCATION, // The "X" nodes whose location in the tree is significant
    BY_VALUE     // The "Y" nodes whose location dous not matter, only subtree "value"
};

typedef TreePtr<Node> Value;
const Value NullValue;

/** Implements a systemic constraint as discussed in #107
 */
class Constraint
{
public:
    /**
     * Get the degree of the constraint.
     * 
     * @return The constraint's degree.
     */
    virtual int GetDegree() const = 0;
    
    /**
     * Get the degree of the constraint.
     * 
     * @return A list of variables affecteed by this constraint. Size equals the return from GetDegree()
     */
    virtual list<VariableId> GetVariables() const = 0;
    
    /**
     * Test a list of variable values for inclusion in the constraint.
     * 
     * @param values [in] the values of the variables, size should be the degree.
     * @param side_info [out] information relating to abnormal contexts etc, only defined if true is returned.
     * 
     * @retval true the values are in the constraint, same ordering as return of GetVariables().
     * @retval false the values are not in the constraint
     */
    virtual bool Test( list< Value > values, 
                       SideInfo *side_info = nullptr ) = 0;        

    /**
     * Given a set of possible values for variable 0, expand the domain
     * to include all possible values for the other variables. This permits
     * "local links" to appear in the domain.
     *
     * @param domain [inout] The domain to expand
     */
    void ExpandDomain( set< TreePtr<Node> > &domain ) { /* not yet implemented */ }

};

};

#endif
