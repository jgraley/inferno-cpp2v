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
    enum class Freedom
    {
        FORCED,
        FREE
    };

    struct VariableFlags
    {
        // TODO Would like to use bitfields but gcc says no https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51242
        Freedom freedom;
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

    enum class Action
    {
        FULL,
        COUPLING
    };

    typedef function< VariableFlags( VariableId ) > VariableQueryLambda;

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
                                 Action action,
                                 VariableQueryLambda vql );
    
private:
    int GetFreeDegree() const;
    list<VariableId> GetFreeVariables() const;
    void TraceProblem() const;
    virtual void Start( const Assignments &forces, 
                        const SR::TheKnowledge *knowledge );    
    bool Test( list< Value > values );
        
    class ByLocationLinkMismatch : public ::Mismatch
    {
    };
    class ByValueLinkMismatch : public ::Mismatch
    {
    };
    
    const struct Plan
    {
        explicit Plan( SR::PatternLink keyer_plink, 
                       set<SR::PatternLink> residual_plinks,             
                       Action action,          
                       VariableQueryLambda vql );
        void RunVariableQueries( VariableQueryLambda vql );
                                 
        const SR::PatternLink keyer_plink;
        const set<SR::PatternLink> residual_plinks;
        const Action action;
        SR::Agent * agent;
        shared_ptr<SR::PatternQuery> pq; // links run over all vars minus agent
        list<VariableRecord> all_variables;
    } plan;
    
    list<Value> forces;
    const SR::TheKnowledge *knowledge;
    
    virtual string GetTrace() const;
};

};

string Trace( const CSP::SystemicConstraint::VariableRecord &var );

#endif
