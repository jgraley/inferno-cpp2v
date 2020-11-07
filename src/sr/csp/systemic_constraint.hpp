#ifndef SYSTEMIC_CONSTRAINT_HPP
#define SYSTEMIC_CONSTRAINT_HPP

#include "constraint.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>
#include <memory>

namespace SR
{
class Conjecture;    
}

namespace CSP
{ 

/** Implements a systemic constraint as discussed in #107
 */
class SystemicConstraint : public Constraint
{
public:
    /**
     * Create the constraint. 
     * 
     * The constraint is permanently associated with the supplied agent.
     * 
     * @param agent the agent from which the constraint will be created.
     * 
     * @param vql callback that requests information about variables
     */
    explicit SystemicConstraint( SR::PatternLink keyer_plink, 
                                 set<SR::PatternLink> residual_plinks,
                                 VariableQueryLambda vql );
    
private:
    int GetFreeDegree() const;
    list<VariableId> GetFreeVariables() const;
    void TraceProblem() const;
    virtual void SetForces( const map<VariableId, Value> &forces );    
    bool Test( list< Value > values );
        
    class ByLocationLinkMismatch : public ::Mismatch
    {
    };
    class ByValueLinkMismatch : public ::Mismatch
    {
    };
    
    enum class Kind
    {
        KEYER,
        RESIDUAL,
        CHILD
    };
    
    struct VariableRecord
    {
        Kind kind;
        VariableId id;
        VariableFlags flags;
    };

    const struct Plan
    {
        explicit Plan( SR::PatternLink keyer_plink, 
                       set<SR::PatternLink> residual_plinks,                       
                       VariableQueryLambda vql );
        void RunVariableQueries( VariableQueryLambda vql );
                                 
        SR::PatternLink keyer_plink;
        set<SR::PatternLink> residual_plinks;
        SR::Agent * agent;
        shared_ptr<SR::PatternQuery> pq; // links run over all vars minus agent
        list<VariableRecord> all_variables;
        shared_ptr<SR::Conjecture> conj;
    } plan;
    
    list<Value> forces; // only FREE vars
    
    virtual string GetTrace() const;
};

};

#endif
