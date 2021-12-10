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
    virtual const list<VariableId> &GetVariables() const = 0;
          
    /**
     * Set the values of the forced varibles
     * 
     * @param forces [in] a map from varaibles to forced values
     */    
    virtual void Start( const Assignments &forces, 
                        const SR::TheKnowledge *knowledge ) = 0;    
    
    /**
     * Test a list of free variable values for inclusion in the constraint. A MisMatch
     * exception is thrown if the assignment does not match.
     * 
     * @param frees_map [in] a partial assignment of free varaibles for this constraint. 
     * All required varaibles should be present. Variables not used by this constraint 
     * are ignored.
     */
    virtual tuple<bool, Assignment> Test( Assignments frees_map,
                                          VariableId current_var ) = 0;        
    
    string GetTrace() const;

    /**
     * Write info about the problem to log via TRACE
     */
    virtual void Dump() const = 0;
};

};

#endif
