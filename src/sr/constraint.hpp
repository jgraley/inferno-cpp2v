#ifndef CONSTRAINT
#define CONSTRAINT

#include "query.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>

namespace SR
{
class Agent;
}

namespace CSP
{ 
struct SideInfo
{
    Set< shared_ptr<const SR::DecidedQuery> > evaluator_queries;   
    Set< pair< shared_ptr<const SR::DecidedQuery>, const SR::DecidedQuery::Link * > > abnormal_links; 
    Set< pair< shared_ptr<const SR::DecidedQuery>, const SR::DecidedQuery::Link * > > multiplicity_links;
};

/**
 * Embed the assumption that variables are simply 1:1 with agents. Note
 * that this class will _always_ take and Agent * in its constructor, 
 * because that's what "systemic constraint" means - and so the constructor
 * takes Agent * explicitly. But the variables could change, so we 
 * introduce a typedef for them.
 */
typedef SR::Agent * VariableId;
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
};

};

#endif