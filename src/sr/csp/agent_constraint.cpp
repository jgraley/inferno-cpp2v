#include "agent_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;


AgentConstraint::Plan::Plan( AgentConstraint *algo_,
                             SR::Agent *agent,
                             set<SR::PatternLink> relevent_plinks,
                             Action action_ ) :
    algo( algo_ ),
    action( action_ ),
    agent( agent )
{
    DetermineVariables( relevent_plinks );       
}


AgentConstraint::AgentConstraint( SR::Agent *agent,
                                  set<SR::PatternLink> relevent_plinks,
                                  Action action ) :
    plan( this, agent, relevent_plinks, action )
{
}


void AgentConstraint::Plan::DetermineVariables( set<SR::PatternLink> relevent_plinks )
{ 
    // The keyer
    SR::PatternLink keyer_plink = agent->GetKeyerPatternLink();
    if( relevent_plinks.count(keyer_plink)==1 )
        variables.push_back( keyer_plink ); 
    
    // The residuals
    set<SR::PatternLink> residual_plinks = agent->GetResidualPatternLinks();
    residual_plinks = IntersectionOf( residual_plinks, relevent_plinks );
    for( VariableId residual_plink : residual_plinks )
        variables.push_back( residual_plink );     
    
    // The children
    if( action==Action::FULL )
    {
        shared_ptr<SR::PatternQuery> pq = agent->GetPatternQuery();
        FOREACH( SR::PatternLink child_plink, pq->GetNormalLinks() )
            if( relevent_plinks.count(child_plink)==1 )
                variables.push_back( child_plink );      
    }
}


string AgentConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &AgentConstraint::GetVariables() const
{ 
    return plan.variables;
}


void AgentConstraint::Start( const Assignments &forces_map_, 
                             const SR::TheKnowledge *knowledge_ )
{
    forces_map = forces_map_;
    knowledge = knowledge_;
    ASSERT( knowledge );
}   


void AgentConstraint::Test( Assignments frees_map )
{   
    INDENT("T");
    //TRACE("Free assignments:\n")
    //     (frees_map)("\n");

    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by NLQ.
    SR::SolutionMap full_map;
    full_map = UnionOfSolo(forces_map, frees_map);

    //TRACE("Now ready to query the agent, with these required links:\n")
    //     (required_links)("\n");

    {
        Tracer::RAIIDisable silencer(); // make queries be quiet

        if( plan.action==Action::FULL || plan.action==Action::COUPLING )
        {
            // First check any coupling at this pattern node
            //TRACE("Coupling query\n");
            plan.agent->RunCouplingQuery( &full_map );
        }
                      
        if( plan.action==Action::FULL )
        {
            //TRACE("Normal linked query\n");
            // Use a normal-linked query on our underlying agent.
            // We only need one match to know that required_links_list are good, 
            // i.e. to run once without throuwing a mismatch. Don't need
            // the returned query.
            plan.agent->RunNormalLinkedQuery( &full_map, knowledge );    
        }
    }            
}


void AgentConstraint::Dump() const
{
    TRACE("Degree %d ", 
          plan.variables.size());
        
    switch( plan.action )
    {
    case Action::FULL:
        TRACEC("FULL ");
        break;
    
    case Action::COUPLING:
        TRACEC("COUPLING ");
        break;
    }

    TRACEC("Agent=")(plan.agent)("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
