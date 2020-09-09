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

// Flags is a bitfield, modulated by flags
enum class CompareBy
{
    LOCATION,
    VALUE
};


// Flags is a bitfield, modulated by flags
enum class Freedom
{
    FORCED,
    FREE
};

typedef TreePtr<Node> Value;

struct ValueFlags
{
    // TODO Would like to use bitfields but gcc says no https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51242
    CompareBy compare_by; 
};


typedef SR::Agent * VariableId;

struct VariableFlags
{
    // TODO Would like to use bitfields but gcc says no https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51242
    CompareBy compare_by; 
    Freedom freedom;
};


typedef function< VariableFlags( VariableId ) > VariableQueryLambda;

void CheckConsistent( VariableId variable, Value value );


/** Implements a systemic constraint as discussed in #107
 */
class Constraint : public Traceable
{
public:
    Constraint() {}
    virtual ~Constraint() {}

    /**
     * Get the degree of the constraint.
     * 
     * @return The constraint's degree.
     */
    virtual int GetFreeDegree() const = 0;
    
    /**
     * Get the degree of the constraint.
     * 
     * @return A list of free variables affecteed by this constraint. Size equals the return from GetFreeDegree()
     */
    virtual list<VariableId> GetFreeVariables() const = 0;
    
    /**
     * Write info about the problem to log via TRACE
     */
    virtual void TraceProblem() const = 0;
        
    /**
     * Set the values of the forced varibles
     * 
     * @param forces [in] a map from varaibles to forced values
     */    
    virtual void SetForces( const map<VariableId, Value> &forces ) = 0;    
    
    /**
     * Test a list of variable values for inclusion in the constraint.
     * 
     * @param values [in] the values of the variables, size should be the degree.
     * @param side_info [out] information relating to abnormal contexts etc, only defined if true is returned.
     * 
     * @retval true the values are in the constraint, same ordering as return of GetFreeVariables().
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
