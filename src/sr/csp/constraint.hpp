#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include "query.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../link.hpp"

#include <memory>
#include <list>
#include <memory>
#include <functional>

namespace SR
{
class Agent;
class TheKnowledge;
};

namespace CSP
{ 
typedef SR::XLink Value;
typedef SR::PatternLink VariableId;
typedef SR::LocatedLink Assignment;
typedef map<VariableId, Value> Assignments;
typedef Assignments Solution;
typedef pair<SR::PatternLink, set<SR::XLink>> Hint;


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
     * Set the values of the forced varibles
     * 
     * @param knowledge [in] pointer to an uptodate TheKnowledge object
     */    
    virtual void Start( const SR::TheKnowledge *knowledge ) = 0;    
    
    /**
     * Test a list of variable values for consistency with this constraint. 
     * 
     * @param assignments [in] a partial assignment of free and forced 
     * varaibles for this constraint. 
     * 
     * @note All required variables should be present. Variables not used 
     * by this constraint are ignored.
     * 
     * @return A tuple of: success (true of consistent) and an optional hint.
     */
    virtual tuple<bool, Hint> Test( const Assignments &assignments,
                                    const VariableId &current_var ) = 0;        
    
    string GetTrace() const;

    /**
     * Write info about the problem to log via TRACE
     */
    virtual void Dump() const = 0;
};

};

#endif
