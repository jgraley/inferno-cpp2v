#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include "query.hpp"

#include "node/node.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>
#include <functional>

namespace SR
{
class Agent;
class BooleanEvaluator;
};

namespace CSP
{ 
struct SideInfo
{
    set< pair< shared_ptr<SR::BooleanEvaluator>, SR::DecidedQuery::Links > > evaluator_records;   
    set< shared_ptr<const SR::DecidedQuery::Link> > abnormal_links; 
    set< shared_ptr<const SR::DecidedQuery::Link> > multiplicity_links;
};

/**
 * Embed the assumption that variables are simply 1:1 with agents. Note
 * that this class will _always_ take and Agent * in its constructor, 
 * because that's what "systemic constraint" means - and so the constructor
 * takes Agent * explicitly. But the variables could change, so we 
 * introduce a typedef for them.
 */
namespace Value
{
enum Category
{
    BY_LOCATION, // The "X" nodes whose location in the tree is significant
    BY_VALUE     // The "Y" nodes whose location dous not matter, only subtree "value"
};

typedef TreePtr<Node> Id;
extern const Id Null;
}

namespace Variable
{

typedef SR::Agent * Id;

// Flags is a bitfield, modulated by flags
enum : int
{
    MASK_BY = 0x01,
    BY_LOCATION = 0x00,
    BY_VALUE = 0x01
};
typedef int Flags;

};

typedef function< Variable::Flags( Variable::Id ) > VariableQueryLambda;

/** Implements a systemic constraint as discussed in #107
 */
class Constraint
{
public:
    Constraint() {}
    virtual ~Constraint() {}

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
    virtual list<Variable::Id> GetVariables() const = 0;
    
    /**
     * Test a list of variable values for inclusion in the constraint.
     * 
     * @param values [in] the values of the variables, size should be the degree.
     * @param side_info [out] information relating to abnormal contexts etc, only defined if true is returned.
     * 
     * @retval true the values are in the constraint, same ordering as return of GetVariables().
     * @retval false the values are not in the constraint
     */
    virtual bool Test( list< Value::Id > values, 
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
