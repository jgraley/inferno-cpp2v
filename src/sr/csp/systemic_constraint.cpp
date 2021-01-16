#include "systemic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;

SystemicConstraint::Plan::Plan( SR::PatternLink keyer_plink_, 
                                set<SR::PatternLink> residual_plinks_,
                                Action action_,
                                VariableQueryLambda vql ) :
    keyer_plink( keyer_plink_ ),
    residual_plinks( residual_plinks_ ),
    action( action_ ),
    agent( keyer_plink.GetChildAgent() ),
    pq( agent->GetPatternQuery() )
{
    for( SR::PatternLink residual_plink : residual_plinks )
        ASSERT( residual_plink.GetChildAgent() == agent );
    
    RunVariableQueries( vql );
}


SystemicConstraint::SystemicConstraint( SR::PatternLink keyer_plink, 
                                        set<SR::PatternLink> residual_plinks,
                                        Action action,
                                        VariableQueryLambda vql ) :
    plan( keyer_plink, residual_plinks, action, vql )
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


int SystemicConstraint::GetFreeDegree() const
{
    int free_degree = 0;
    for( auto var : plan.all_variables )
    {
        if( var.flags.freedom == Freedom::FREE )
            free_degree++;
    }
    return free_degree;
}


list<VariableId> SystemicConstraint::GetFreeVariables() const
{ 
    list<VariableId> free_vars;
    for( auto var : plan.all_variables )
    {
        if( var.flags.freedom == Freedom::FREE )
            free_vars.push_back( var.id );
    }
    return free_vars;
}


list<VariableId> SystemicConstraint::GetRequiredVariables() const
{ 
    list<VariableId> free_vars;
    for( auto var : plan.all_variables )
    {
        // We require the free keyer if there is one (otherwise it will be a force)
        if( var.flags.freedom != Freedom::FREE )
            continue;
             
        switch( var.kind )
        {
        case Kind::KEYER:
            free_vars.push_back( var.id );
            break;
        case Kind::RESIDUAL:
            break;
        case Kind::CHILD:
            break;
        }        
    }
    return free_vars;
}


void SystemicConstraint::TraceProblem() const
{
    TRACEC("SystemicConstraint ")(*this)(" degree %d free degree %d\n", plan.all_variables.size(), GetFreeDegree());
    for( auto var : plan.all_variables )
    {
        TRACEC(Trace(var))("\n");
    }    
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
                  (Trace(var))(" should not be in forces: ")(forces_map)("\n");
            break;
            
        case Freedom::FORCED:
            ASSERT( forces_map.count( var.id ) == 1 )
                  (*this)("\n")
                  (Trace(var))(" missing from forces: ")(forces_map)("\n"); 
            forces.push_back( forces_map.at( var.id ) );            
            break;
        }
    }    
}   


void SystemicConstraint::Test( Assignments frees_map )
{   
    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by NLQ.
    Value x;
    SR::SolutionMap required_links;
    multiset<SR::XLink> coupling_links;
    list<Value>::const_iterator forceit = forces.begin();
    for( auto var : plan.all_variables )
    {
        Value v;
        switch( var.flags.freedom )
        {
        case Freedom::FORCED:
            v = forces_map.at(var.id);
            break;
        
        case Freedom::FREE:
            if( frees_map.count(var.id)==0 )
                continue; // value not supplied
            v = frees_map.at(var.id);
            break;
        }
        
        switch( var.kind )
        {
        case Kind::KEYER:
            x = v;
            coupling_links.insert(v);
            break;
        
        case Kind::RESIDUAL:
            coupling_links.insert(v);
            break;
            
        case Kind::CHILD:
            required_links[var.id] = v;     
            break;
        }
    }    
    ASSERT( x ); // The keyer is required
    //required_links = UnionOfSolo(forces_map, frees_map);
    
    {
        Tracer::RAIIEnable silencer( false ); // make queries be quiet

        if( plan.action==Action::FULL || plan.action==Action::COUPLING )
        {
            // First check any coupling at this pattern node
            plan.agent->RunCouplingQuery( coupling_links );
        }
              
        if( plan.action==Action::FULL )
        {
            // Use a normal-linked query on our underlying agent.
            // We only need one match to know that required_links_list are good, 
            // i.e. to run once without throuwing a mismatch. Don't need
            // the returned query.
            plan.agent->RunNormalLinkedQuery( x, &required_links, knowledge );    
        }
    }            
}


string SystemicConstraint::GetTrace() const
{
    string s = string("SystemicConstraint(");
    
    s += plan.agent->GetTrace() + ": ";
    
    switch( plan.action )
    {
    case Action::FULL:
        s += "FULL";
        break;
    
    case Action::COUPLING:
        s += "COUPLING";
        break;
    }

    s += SSPrintf(" req=%d/%d", GetRequiredVariables().size(), GetFreeVariables().size());

    s += ")";
    return s;
}      


string Trace( const SystemicConstraint::VariableRecord &var )
{
    string skind;
    switch( var.kind )
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
    switch( var.flags.freedom )
    {
    case SystemicConstraint::Freedom::FREE:
        sfreedom = "FREE";
        break;
        
    case SystemicConstraint::Freedom::FORCED:
        sfreedom = "FORCED";
        break;
    }
        
    return skind+":"+var.id.GetTrace()+"("+sfreedom+")";
}

