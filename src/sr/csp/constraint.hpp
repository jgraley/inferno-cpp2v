#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

//#include "query.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../link.hpp"
#include "../sym/expression.hpp"

#include <memory>
#include <list>
#include <memory>
#include <functional>

namespace SR
{
class Agent;
};

namespace SYM
{
class SetResult;
};

namespace CSP
{ 
typedef SR::XLink Value;
typedef SR::PatternLink VariableId;
typedef SR::LocatedLink Assignment;
typedef map<VariableId, Value> Assignments;
typedef Assignments Solution;
typedef pair<VariableId, set<Value>> Hint;


/** Implements a systemic constraint as discussed in #107
 */
class Constraint : public Traceable,
                   public SerialNumber
{
public:
    Constraint() {}
    virtual ~Constraint() {}
  
    /**
     * Get the free variables.
     * 
     * @return A list of variables affected by this constraint.
     */
    virtual const set<VariableId> &GetVariables() const = 0;
          
    /**
     * Get the amount of x_tree_db this constraint's methods will require.
     * 
     * @return A list of variables affected by this constraint.
     */
    virtual SYM::Expression::VariablesRequiringNuggets GetVariablesRequiringNuggets() const = 0;

    /**
     * Set the values of the forced varibles
     * 
     * @param x_tree_db [in] pointer to an uptodate XTreeDatabase object
     */    
    virtual void Start() = 0;    
    
    /**
     * Determine whether a partila assignment satisfies this constraint. 
     * 
     * @param assignments [in] a partial assignment of free and forced 
     * varaibles for this constraint. 
     * 
     * @note All required variables should be present. Variables not used 
     * by this constraint are ignored.
     * 
     * @return True if satisfied.
     */
    virtual bool IsSatisfied( const Assignments &assignments ) const = 0;        
    
    /**
     * Get a set of values for a given variable that contains all satisfying values. 
     * 
     * @param assignments [in] a partial assignment of free and forced 
     * varaibles for this constraint. 
     * 
     * @param var [in] the variables to get suggested values for. Must be a free
     * variable of this constraint.
     * 
     * @return A symbolic result which, as a set, should contain all suggested values
     * 
     * @note the returned set is allowed to contain inconsistent values. 
     */
    virtual unique_ptr<SYM::SetResult> GetSuggestedValues( const Assignments &assignments,
                                                           const VariableId &var ) const = 0;        
    
    string GetTrace() const;

    /**
     * Write info about the problem to log via TRACE
     */
    virtual void Dump() const = 0;
};

};

#endif
