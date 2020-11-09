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
class BooleanEvaluator;
};

namespace CSP
{ 
typedef SR::XLink Value;
typedef SR::PatternLink VariableId;

void CheckLocalMatch( VariableId variable, Value value );


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
     * Test a list of free variable values for inclusion in the constraint.
     * 
     * @param values [in] the values of the variables, size should be the degree.
     * @param side_info [out] information relating to abnormal contexts etc, only defined if true is returned.
     * 
     * @retval true the values are in the constraint, same ordering as return of GetFreeVariables().
     * @retval false the values are not in the constraint
     */
    virtual bool Test( list< Value > values ) = 0;        
};

};

#endif
