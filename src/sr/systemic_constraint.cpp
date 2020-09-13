#include "systemic_constraint.hpp"
#include "query.hpp"
#include "agent.hpp"
#include "helpers/simple_compare.hpp"

using namespace CSP;

SystemicConstraint::Plan::Plan( SR::Agent *agent_, 
                                VariableQueryLambda vql ) :
    agent( agent_ ),
    pq( agent->GetPatternQuery() ),
    simple_compare( make_shared<SimpleCompare>() )
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


SystemicConstraint::SystemicConstraint( SR::Agent *agent_, 
                                        VariableQueryLambda vql ) :
    plan( agent_, vql )
{
}

    
void SystemicConstraint::Plan::GetAllVariables()
{
    all_variables.clear();
    
    all_variables.push_back( agent ); // The me-variable
    
    FOREACH( SR::PatternLink link, pq->GetAllLinks() )
    {
        VariableId var = link.GetChildAgent();
        all_variables.push_back( var );  
    }
}


void SystemicConstraint::Plan::RunVariableQueries( list<VariableId> vars, VariableQueryLambda vql )
{ 
    diversions.clear();
    flags.clear();
    for( VariableId v : vars )
    {
        pair<VariableFlags, VariableId> r = vql( v );
        flags.push_back( r.first );
        switch( r.first.correspondance )
        {
        case Correspondance::DIRECT:
            ASSERT( r.second == nullptr );
            break;
            
        case Correspondance::DIVERTED:
            ASSERT( r.second != v );
            diversions[v] = r.second;
            break;
        }
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
        if( plan.diversions.count(var) > 0 )
            var = plan.diversions.at(var);

        if( fit->freedom == Freedom::FREE )
            free_vars.push_back( var );
        fit++;
    }
    return free_vars;
}


void SystemicConstraint::TraceProblem() const
{
    TRACEC("SystemicConstraint degree %d free degree %d\n", plan.flags.size(), GetFreeDegree());
    INDENT(" ");
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
            
        case CompareBy::VALUE:
            sflags += "VALUE";
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
        sflags += " and ";
        switch( fit->correspondance )
        {
        case Correspondance::DIRECT:
            sflags += "DIRECT";
            break;
            
        case Correspondance::DIVERTED:
            sflags += "DIVERTED";
            break;
        }
        fit++;
        
        TRACEC(*var)(scat)(sflags)("\n");
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
    // values that must tally up with the DQ links.
    TreePtr<Node> x;
    list< Value > expanded_values;
    auto fit = forces.begin();
    auto valit = values.begin();
    bool first = true; // First flag refers to the me-variable
    for( const VariableFlags &f : plan.flags )
    {
        switch( f.freedom )
        {
        case Freedom::FORCED:
            if( first )         
                x = *fit;
            else
                expanded_values.push_back( *fit );
            fit++;
            break;
        
        case Freedom::FREE:
            if( first )             
                x = values.front();
            else
                expanded_values.push_back( *valit );
            valit++;
            break;
        }
        first = false;
    }    
    
    // We're going to be lazy and borrow the Conjecture class for now.
    // Our constructor has already configured it to have our agent and
    // only our agent, so we won't really be using the multi-agent support
    // the Conjecture offers. We really just want the algorithm out of
    // Conjecture::IncrementAgent().
    //
    // Prepare the conjecture for a series of iterations.
    plan.conj->Start();
    shared_ptr<SR::DecidedQuery> query; 
    
    // All the other values are normal links. These degrees of freedom
    // will be a mixture of (a) depending on our our decisions or (b)
    // fixed single value resulting from x.
    
    // This block resembles AndRuleEngine::Compare() (the big one) because
    // it walks the conjecture through to success (match) or failure (out
    // of options).
    while(1)
    {
        // Try block catches mismatches which are thrown as exceptions
        try
        {
            {
                Tracer::RAIIEnable silencer( false ); // make DQ be quiet
                // Similar to AndRuleEngine::DecidedCompare(), we get the
                // Query object from conjecture, and run a query on it.
                query = plan.conj->GetQuery(plan.agent);
                plan.agent->RunDecidedQuery( *query, x );
            }
            
            // The query now has populated links, which should be full
            // (otherwise RunDecidedQuery() (DQ) should have thrown). We loop 
            // over both and check that they refer to the same x nodes
            // we were passed. Mismatch will throw, same as in DQ.
            auto links = query->GetAllLinks();
            ASSERT( links.size() == expanded_values.size() );
            auto linkit = links.begin();      
            auto fit = plan.flags.begin();      
            fit++; // skip "me"
            for( TreePtr<Node> val : expanded_values )
            {
                Value x = linkit->GetChildX();
                switch( fit->compare_by )
                {
                case CompareBy::VALUE:
                    if( !(*plan.simple_compare)( val, x ) )
                        throw ByValueLinkMismatch();  
                    break;
                    
                case CompareBy::LOCATION:
                    if( val != x )
                        throw ByLocationLinkMismatch();  
                    break;
                }
                // Not expected to pass. We don't check whether the link values
                // we generate from our given base value are compatible with
                // the child agent, because that the job of that agent's 
                // constraint.
                //CheckLocalMatch( (*linkit)->agent, val );   
                linkit++;
                fit++;
            }
        }
        catch( ::Mismatch & )
        {
            Tracer::RAIIEnable silencer( false ); // make DQ be quiet
            // We will get here on a mismatch, whether detected by DQ or our
            // own comparison between links and values. Permit the conjecture
            // to move to a new set of choices.
            if( plan.conj->Increment() )
                continue; // Conjecture would like us to try again with new choices
            
            // Conjecture has run out of choices to try.
            return false; 
        }            
        break; // Didn't throw: success
    }    

    return true;
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
                    plan.agent->RunDecidedQuery( *query, x );
                    FOREACH( const SR::LocatedLink &link, *query->GetAllLinks() )
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
        
        SimpleCompare sc;
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