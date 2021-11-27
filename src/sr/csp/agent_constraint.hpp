#ifndef AGENT_CONSTRAINT_HPP
#define AGENT_CONSTRAINT_HPP

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
class AgentConstraint : public Constraint
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
    
    struct VariableRecord : Traceable
    {
        VariableRecord( Kind kind_,
                        VariableId id_,
                        VariableFlags flags_ );
        string GetTrace() const;

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
    explicit AgentConstraint( SR::Agent *agent,
                              set<SR::PatternLink> relevent_residuals,
                              Action action,
                              VariableQueryLambda vql );
    
private:
    const list<VariableId> &GetFreeVariables() const;
    virtual void Start( const Assignments &forces_map_, 
                        const SR::TheKnowledge *knowledge_ );    
    void Test( Assignments frees_map );
        
    class ByLocationLinkMismatch : public ::Mismatch
    {
    };
    class ByValueLinkMismatch : public ::Mismatch
    {
    };
    
    const struct Plan : public virtual Traceable
    {
        explicit Plan( AgentConstraint *algo,  
                       SR::Agent *agent,
                       set<SR::PatternLink> relevent_residuals,       
                       Action action,          
                       VariableQueryLambda vql );
        void RunVariableQueries( set<SR::PatternLink> relevent_residuals, 
                                 VariableQueryLambda vql );
        string GetTrace() const; // used for debug

        AgentConstraint * const algo;
        const Action action;
        SR::Agent * agent;
        shared_ptr<SR::PatternQuery> pq; // links run over all vars minus agent
        list<VariableRecord> all_variables;
        int free_degree;
        list<VariableId> free_variable_ids;
    } plan;
    
    list<Value> forces;
    Assignments forces_map;
    const SR::TheKnowledge *knowledge;
    
    void Dump() const;
};

};

#endif
