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

    // Configure our embedded Conjecture object
    set<SR::Agent *> my_agents;
    my_agents.insert( agent ); // just the one agent this time
    conj = make_shared<SR::Conjecture>(my_agents, agent);    
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
    all_variables.clear();
    
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
    FOREACH( SR::PatternLink child_plink, pq->GetNormalLinks() )
        all_variables.push_back( VariableRecord{ Kind::CHILD, 
                                                 child_plink, 
                                                 vql(child_plink) } );      
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


void SystemicConstraint::TraceProblem() const
{
    TRACEC("SystemicConstraint ")(*this)(" degree %d free degree %d\n", plan.all_variables.size(), GetFreeDegree());
    for( auto var : plan.all_variables )
    {
        string scat;
        switch( var.kind )
        {
        case Kind::KEYER:
            scat = "KEYER ";
            break;
        
        case Kind::RESIDUAL:
            scat = "RESIDUAL ";
            break;
            
        case Kind::CHILD:
            scat = "CHILD ";
            break;
        }

        string sflags = "; is ";
        switch( var.flags.freedom )
        {
        case Freedom::FREE:
            sflags += "FREE";
            break;
            
        case Freedom::FORCED:
            sflags += "FORCED";
            break;
        }
        
        TRACEC(scat)(var.id)(sflags)("\n");
    }    
}


void SystemicConstraint::SetForces( const map<VariableId, Value> &forces_ )
{
    forces.clear();
    for( auto var : plan.all_variables )
    {
        switch( var.flags.freedom )
        {
        case Freedom::FREE:
            break;
            
        case Freedom::FORCED:
            forces.push_back( forces_.at( var.id ) );
            break;
        }
    }    
}   


bool SystemicConstraint::Test( list< Value > values )
{
    ASSERT( values.size() == GetFreeDegree() );
    
    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by NLQ.
    Value x;
    list<SR::LocatedLink> required_links;
    multiset<SR::XLink> coupling_links;
    list<Value>::const_iterator forceit = forces.begin();
    list<Value>::const_iterator valit = values.begin();
    list<VariableId>::const_iterator patit = plan.pq->GetNormalLinks().begin();
    for( auto var : plan.all_variables )
    {
        Value v;
        switch( var.flags.freedom )
        {
        case Freedom::FORCED:
            v = *forceit++;
            break;
        
        case Freedom::FREE:
            v = *valit++;
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
            required_links.push_back( SR::LocatedLink(*patit++, v) );     
            break;
        }
    }    

    try
    {
        if( plan.action==Action::FULL || plan.action==Action::COUPLING )
        {
            // First check any coupling at this pattern node
            plan.agent->CouplingQuery( coupling_links );
        }
              
        if( plan.action==Action::FULL )
        {
            // Use a normal-linked query on our underlying agent   
            shared_ptr<SR::DecidedQuery> query = plan.agent->CreateDecidedQuery();
            plan.agent->RunNormalLinkedQuery( query, x, required_links );      
        }

        return true;
    }
    catch( ::Mismatch & )
    {
        // CouplingQuery() or RunNormalLinkedQuery() couldn't match.
        return false; 
    }               
}


string SystemicConstraint::GetTrace() const
{
    return string("SystemicConstraint(") + plan.agent->GetTrace() + ")";
}      







#if 0
// Some domain extension code, see #120

        list< TreePtr<Node> > x_to_add;
        Conjecture lconj;
        set<SR::Agent *> lagents;
        lagents.insert( plan.agent ); // just the one agent this time
        lconj.Configure(lagents, plan.agent);
        for( TreePtr<Node> x : x_nodes )
        {
            lconj.Start();
            while(1)
            {
                try
                {
                    query = lconj.GetQuery(plan.agent);
                    plan.agent->RunDecidedQuery( *query, XLink(x) );
                    FOREACH( const SR::LocatedLink &link, *query->GetNormalLinks() )
                    x_to_add.push_back(x);
                }
                catch( ::Mismatch & )
                {                    
                    if( conj->Increment() )
                        continue; // Conjecture would like us to try again with new choices                    
                    break; 
                }            
            }
        }
        
        EquivalenceRelation sc;
        while( !x_to_add.empty() )
        {
            bool found = false;
            for( TreePtr<Node> x : x_nodes )
            {
                try
                {
                    sc( x, x_to_add.front() );
                    found = true;
                    break;
                }
                catch( ::Mismatch & )
                {
                }                
            }
            if( !found )
            
        }

#endif