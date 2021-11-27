#include "systemic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;

SystemicConstraint::VariableRecord::VariableRecord( Kind kind_,
                                                    VariableId id_,
                                                    VariableFlags flags_ ) :
    kind( kind_ ),
    id( id_ ),
    flags( flags_ )
{
}


string SystemicConstraint::VariableRecord::GetTrace() const
{
    string skind;
    switch( kind )
    {
    case SystemicConstraint::Kind::KEYER:
        skind = "KEYER";
        break;
    
    case SystemicConstraint::Kind::RESIDUAL:
        skind = "RESIDUAL";
        break;
        
    case SystemicConstraint::Kind::CHILD:
        skind = "CHILD";
        break;
    }

    string sfreedom;
    switch( flags.freedom )
    {
    case SystemicConstraint::Freedom::FREE:
        sfreedom = "FREE";
        break;
        
    case SystemicConstraint::Freedom::FORCED:
        sfreedom = "FORCED";
        break;
    }
        
    return skind+":"+id.GetTrace()+"("+sfreedom+")";
}


SystemicConstraint::Plan::Plan( SystemicConstraint *algo_,
                                SR::PatternLink keyer_plink_, 
                                set<SR::PatternLink> residual_plinks_,
                                Action action_,
                                VariableQueryLambda vql ) :
    algo( algo_ ),
    keyer_plink( keyer_plink_ ),
    residual_plinks( residual_plinks_ ),
    action( action_ ),
    agent( keyer_plink.GetChildAgent() ),
    pq( agent->GetPatternQuery() )
{
    for( SR::PatternLink residual_plink : residual_plinks )
        ASSERT( residual_plink.GetChildAgent() == agent );
    
    RunVariableQueries( vql );
    
    for( auto var : all_variables )
    {
        // Only need to report FREE variables as being required
        if( var.flags.freedom != Freedom::FREE )
            continue;
             
        free_variable_ids.push_back( var.id );     
    }
}


SystemicConstraint::SystemicConstraint( SR::PatternLink keyer_plink, 
                                        set<SR::PatternLink> residual_plinks,
                                        Action action,
                                        VariableQueryLambda vql ) :
    plan( this, keyer_plink, residual_plinks, action, vql )
{
}


void SystemicConstraint::Plan::RunVariableQueries( VariableQueryLambda vql )
{ 
    // The keyer
    all_variables.push_back( VariableRecord{ Kind::KEYER, 
                                             keyer_plink, 
                                             vql(keyer_plink) } ); 
    
    // The residuals
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


string SystemicConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &SystemicConstraint::GetFreeVariables() const
{ 
    return plan.free_variable_ids;
}


const list<VariableId> &SystemicConstraint::GetRequiredFreeVariables() const
{ 
    return plan.required_free_variable_ids;
}


void SystemicConstraint::Start( const Assignments &forces_map_, 
                                const SR::TheKnowledge *knowledge_ )
{
    forces_map = forces_map_;
    knowledge = knowledge_;
    ASSERT( knowledge );
    
    forces.clear();
    for( auto var : plan.all_variables )
    {
        switch( var.flags.freedom )
        {
        case Freedom::FREE:
            ASSERT( forces_map.count( var.id ) == 0 )
                  (*this)("\n")
                  (var)(" should not be in forces: ")(forces_map)("\n");
            break;
            
        case Freedom::FORCED:
            ASSERT( forces_map.count( var.id ) == 1 )
                  (*this)("\n")
                  (var)(" missing from forces: ")(forces_map)("\n"); 
            forces.push_back( forces_map.at( var.id ) );            
            break;
        }
    }    
}   


void SystemicConstraint::Test( Assignments frees_map )
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


void SystemicConstraint::Dump() const
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
    TRACEC("Required free var ids: ")(plan.required_free_variable_ids)("\n");
}      
