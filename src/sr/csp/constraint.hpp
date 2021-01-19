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
typedef unordered_map<VariableId, Value> Assignments;
typedef Assignments Solution;


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
     * Get the free variables.
     * 
     * @return A list of free variables affecteed by this constraint. Size equals the return from GetFreeDegree()
     */
    virtual list<VariableId> GetFreeVariables() const = 0;
    
    /**
     * Get the free variables that must be assigned when calling Test.
     * 
     * @return A list of free variables required by Test().
     */
    virtual list<VariableId> GetRequiredVariables() const = 0;

    /**
     * Write info about the problem to log via TRACE
     */
    virtual void TraceProblem() const = 0;
        
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
    virtual void Test( Assignments frees_map ) = 0;        
};

};

#endif
