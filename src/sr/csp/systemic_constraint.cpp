#include "systemic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

using namespace CSP;

SystemicConstraint::Plan::Plan( SR::PatternLink root_link_, 
                                VariableQueryLambda vql ) :
    root_link( root_link_ ),
    agent( root_link.GetChildAgent() ),
    pq( agent->GetPatternQuery() )
{
    GetAllVariables();
    RunVariableQueries( all_variables, vql );
    
    // This implementation doesn't know how to model the first varaible
    // by any means other than by location (i.e. by address)
    ASSERT( flags.front().compare_by == CompareBy::LOCATION );

    // Configure our embedded Conjecture object
    set<SR::Agent *> my_agents;
    my_agents.insert( agent ); // just the one agent this time
    conj = make_shared<SR::Conjecture>(my_agents, agent);    
}


SystemicConstraint::SystemicConstraint( SR::PatternLink root_link, 
                                        VariableQueryLambda vql ) :
    plan( root_link, vql )
{
}

    
void SystemicConstraint::Plan::GetAllVariables()
{
    all_variables.clear();
    
    all_variables.push_back( root_link ); // The me-variable
    
    FOREACH( SR::PatternLink link, pq->GetNormalLinks() )
    {
        VariableId var = link;
        all_variables.push_back( var );  
    }
}


void SystemicConstraint::Plan::RunVariableQueries( list<VariableId> vars, VariableQueryLambda vql )
{ 
    flags.clear();
    for( VariableId v : vars )
    {
        VariableFlags r = vql( v );
        flags.push_back( r );
    }
}


int SystemicConstraint::GetFreeDegree() const
{
    int free_degree = 0;
    for( auto f : plan.flags )
    {
        if( f.freedom == Freedom::FREE )
            free_degree++;
    }
    return free_degree;
}


list<VariableId> SystemicConstraint::GetFreeVariables() const
{ 
    list<VariableId> free_vars;
    auto fit = plan.flags.begin();
    for( auto var : plan.all_variables )
    {
        if( fit->freedom == Freedom::FREE )
            free_vars.push_back( var );
        fit++;
    }
    return free_vars;
}


void SystemicConstraint::TraceProblem() const
{
    TRACEC("SystemicConstraint ")(*this)(" degree %d free degree %d\n", plan.flags.size(), GetFreeDegree());
    INDENT("T");
    auto fit = plan.flags.begin();
    bool first = true;
    for( auto var : plan.all_variables )
    {
        string scat = " ";
        if( first )
            scat += "(base)";
        else
            scat += "(link)";
        first = false;    
        
        string sflags;
        sflags += "; compare by ";
        switch( fit->compare_by )
        {
        case CompareBy::LOCATION:
            sflags += "LOCATION";
            break;
            
        case CompareBy::EQUIVALENCE:
            sflags += "EQUIVALENCE";
            break;
        }
        sflags += "; is ";
        switch( fit->freedom )
        {
        case Freedom::FREE:
            sflags += "FREE";
            break;
            
        case Freedom::FORCED:
            sflags += "FORCED";
            break;
        }
        fit++;
        
        TRACEC(var)(scat)(sflags)("\n");
    }
    
}


void SystemicConstraint::SetForces( const map<VariableId, Value> &forces_ )
{
    forces.clear();
    auto fit = plan.flags.begin();
    for( auto var : plan.all_variables )
    {
        switch( fit->freedom )
        {
        case Freedom::FREE:
            break;
            
        case Freedom::FORCED:
            forces.push_back( forces_.at( var ) );
            break;
        }
        fit++;
    }    
}   


bool SystemicConstraint::Test( list< Value > values )
{
    ASSERT( values.size() == GetFreeDegree() );
    
    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by NLQ.
    TreePtr<Node> x;
    list<SR::LocatedLink> expanded_links;
    auto forceit = forces.begin();
    auto valit = values.begin();
    auto patit = plan.pq->GetNormalLinks().begin();
    bool first = true; // First flag refers to the self-variable
    for( const VariableFlags &f : plan.flags )
    {
        switch( f.freedom )
        {
        case Freedom::FORCED:
            if( first )         
                x = *forceit;
            else
                expanded_links.push_back( SR::LocatedLink(*patit, *forceit) );     
            forceit++;
            break;
        
        case Freedom::FREE:
            if( first )             
                x = *valit;
            else
                expanded_links.push_back( SR::LocatedLink(*patit, *valit) );            
            valit++;
            break;
        }
        
        if( !first )
            patit++;
        first = false;
    }    

    // Use a normal-linked query on our underlying agent
    shared_ptr<SR::DecidedQuery> query = plan.agent->CreateDecidedQuery();
    try
    {
        plan.agent->RunNormalLinkedQuery( query, &x, expanded_links );      
    }
    catch( ::Mismatch & )
    {
        // RunNormalLinkedQuery() couldn't match.
        return false; 
    }               

    return true;
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