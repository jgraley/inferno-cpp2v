#include "agent_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;

AgentConstraint::VariableRecord::VariableRecord( Kind kind_,
                                                 VariableId id_,
                                                 VariableFlags flags_ ) :
    kind( kind_ ),
    id( id_ ),
    flags( flags_ )
{
}


string AgentConstraint::VariableRecord::GetTrace() const
{
    string skind;
    switch( kind )
    {
    case AgentConstraint::Kind::KEYER:
        skind = "KEYER";
        break;
    
    case AgentConstraint::Kind::RESIDUAL:
        skind = "RESIDUAL";
        break;
        
    case AgentConstraint::Kind::CHILD:
        skind = "CHILD";
        break;
    }

    string sfreedom;
    switch( flags.freedom )
    {
    case AgentConstraint::Freedom::FREE:
        sfreedom = "FREE";
        break;
        
    case AgentConstraint::Freedom::FORCED:
        sfreedom = "FORCED";
        break;
    }
        
    return skind+":"+id.GetTrace()+"("+sfreedom+")";
}


AgentConstraint::Plan::Plan( AgentConstraint *algo_,
                             SR::Agent *agent,
                             set<SR::PatternLink> feasible_residuals,
                             Action action_,
                             VariableQueryLambda vql ) :
    algo( algo_ ),
    action( action_ ),
    agent( agent ),
    pq( agent->GetPatternQuery() )
{
    RunVariableQueries( feasible_residuals, vql );
    
    for( auto var : all_variables )
    {
        if( var.flags.freedom == Freedom::FREE )
            free_variable_ids.push_back( var.id );     
    }
}


AgentConstraint::AgentConstraint( SR::Agent *agent,
                                  set<SR::PatternLink> feasible_residuals,
                                  Action action,
                                  VariableQueryLambda vql ) :
    plan( this, agent, feasible_residuals, action, vql )
{
}


void AgentConstraint::Plan::RunVariableQueries( set<SR::PatternLink> feasible_residuals,
                                                VariableQueryLambda vql )
{ 
    // The keyer
    SR::PatternLink keyer_plink = agent->GetKeyerPatternLink();
    all_variables.push_back( VariableRecord{ Kind::KEYER, 
                                             keyer_plink, 
                                             vql(keyer_plink) } ); 
    
    // The residuals
    set<SR::PatternLink> residual_plinks = agent->GetResidualPatternLinks();
    residual_plinks = IntersectionOf( residual_plinks, feasible_residuals );
    for( VariableId residual_plink : residual_plinks )
        all_variables.push_back( VariableRecord{ Kind::RESIDUAL, 
                                                 residual_plink, 
                                                 vql(residual_plink) } );     
    
    // The children
    if( action==Action::FULL )
    {
        FOREACH( SR::PatternLink child_plink, pq->GetNormalLinks() )
            all_variables.push_back( VariableRecord{ Kind::CHILD, 
                                                     child_plink, 
                                                     vql(child_plink) } );      
    }
}


string AgentConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &AgentConstraint::GetFreeVariables() const
{ 
    return plan.free_variable_ids;
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
          plan.all_variables.size());
        
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
    TRACEC("Variables: ")(plan.all_variables)("\n");
    TRACEC("Free var ids: ")(plan.free_variable_ids)("\n");
}      
