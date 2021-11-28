#ifndef AGENT_CONSTRAINT_HPP
#define AGENT_CONSTRAINT_HPP

#include "constraint.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"

#include <memory>
#include <list>

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
    enum class Action
    {
        FULL,
        COUPLING
    };

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
                              set<SR::PatternLink> relevent_plinks,
                              Action action );
    
private:
    const list<VariableId> &GetVariables() const;
    virtual void Start( const Assignments &forces_map_, 
                        const SR::TheKnowledge *knowledge_ );    
    void Test( Assignments frees_map );
        
    const struct Plan : public virtual Traceable
    {
        explicit Plan( AgentConstraint *algo,  
                       SR::Agent *agent,
                       set<SR::PatternLink> relevent_plinks,       
                       Action action );
        void DetermineVariables( set<SR::PatternLink> relevent_plinks );
        string GetTrace() const; // used for debug

        AgentConstraint * const algo;
        const Action action;
        SR::Agent * agent;
        list<VariableId> variables;
    } plan;
    
    list<Value> forces;
    Assignments forces_map;
    const SR::TheKnowledge *knowledge;
    
    void Dump() const;
};

};

#endif
