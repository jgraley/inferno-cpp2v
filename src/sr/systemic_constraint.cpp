#include "systemic_constraint.hpp"
#include "query.hpp"
#include "agent.hpp"
#include "helpers/simple_compare.hpp"

using namespace CSP;

SystemicConstraint::SystemicConstraint( SR::Agent *agent_, 
                                        VariableQueryLambda vql ) :
    agent( agent_ ),
    pq( agent->GetPatternQuery() ),
    flags( GetFlags( GetVariablesImpl( agent, pq ), vql ) ),
    conj( make_shared<SR::Conjecture>() ),
    simple_compare( make_shared<SimpleCompare>() )
{    
    // This implementation doesn't know how to model the first varaible
    // by any means other than by location (i.e. by address)
    ASSERT( flags.front().compare_by == CompareBy::LOCATION );
    
    // Configure our embedded Conjecture object
    set<SR::Agent *> my_agents;
    my_agents.insert( agent ); // just the one agent this time
    conj->Configure(my_agents, agent);    
}


list<VariableId> SystemicConstraint::GetVariablesImpl( SR::Agent * const agent, 
                                                       shared_ptr<SR::PatternQuery> pq )
{
    list<VariableId> vars;
    
    vars.push_back( agent ); // The me-variable
    
    FOREACH( shared_ptr<const SR::PatternQuery::Link> b, *pq->GetNormalLinks() )
        vars.push_back( b->agent );  // This rest are our normal linked agents
        
    return vars;
}


list<VariableFlags> SystemicConstraint::GetFlags( list<VariableId> vars, VariableQueryLambda vql )
{
    list<VariableFlags> flags;
    for( VariableId v : vars )
        flags.push_back( vql( v ) );
    return flags;
}


int SystemicConstraint::GetFreeDegree() const
{
    int free_degree = 0;
    for( auto f : flags )
    {
        if( f.freedom == Freedom::FREE )
            free_degree++;
    }
    return free_degree;
}


list<VariableId> SystemicConstraint::GetFreeVariables() const
{ 
    list<VariableId> vars = GetVariablesImpl(agent, pq); 
    list<VariableId> free_vars;
    auto fit = flags.begin();
    for( auto var : vars )
    {
        if( fit->freedom == Freedom::FREE )
            free_vars.push_back( var );
        fit++;
    }
    return free_vars;
}


void SystemicConstraint::TraceProblem() const
{
    TRACEC("SystemicConstraint degree %d free degree %d\n", flags.size(), GetFreeDegree());
    INDENT(" ");
    list<VariableId> vars = GetVariablesImpl(agent, pq); 
    auto fit = flags.begin();
    bool first = true;
    for( auto var : vars )
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
        fit++;
        
        TRACEC(*var)(scat)(sflags)("\n");
    }
    
}


void SystemicConstraint::SetForces( const map<VariableId, Value> &forces_ )
{
    list<VariableId> vars = GetVariablesImpl(agent, pq); 
    forces.clear();
    auto fit = flags.begin();
    for( auto var : vars )
    {
        if( fit->freedom == Freedom::FORCED )
            forces.push_back( forces_.at( var ) );
        fit++;
    }    
}   


bool SystemicConstraint::Test( list< Value > values,
                               SideInfo *side_info )
{
    ASSERT( values.size() == GetFreeDegree() );
    for( Value v : values )
    {
        ASSERT( v != NullValue );
    }
    
    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the DQ links.
    TreePtr<Node> x;
    list< Value > expanded_values;
    auto fit = forces.begin();
    auto valit = values.begin();
    bool first = true; // First flag refers to the me-variable
    for( const VariableFlags &f : flags )
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
    conj->Start();
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
                query = conj->GetQuery(agent);
                agent->RunDecidedQuery( *query, x );
            }
            
            // The query now has populated links, which should be full
            // (otherwise RunDecidedQuery() (DQ) should have thrown). We loop 
            // over both and check that they refer to the same x nodes
            // we were passed. Mismatch will throw, same as in DQ.
            auto links = query->GetNormalLinks();
            ASSERT( links->size() == expanded_values.size() );
            auto linkit = links->begin();      
            auto fit = flags.begin();      
            fit++; // skip "me"
            for( TreePtr<Node> val : expanded_values )
            {
                switch( fit->compare_by )
                {
                case CompareBy::VALUE:
                    if( !(*simple_compare)( val, (*linkit)->x ) )
                        throw ByValueLinkMismatch();  
                    break;
                    
                case CompareBy::LOCATION:
                    if( val != (*linkit)->x )
                        throw ByLocationLinkMismatch();  
                    break;
                }
                // Not expected to pass. We don't check whether the link values
                // we generate from our given base value are compatible with
                // the child agent, because that the job of that agent's 
                // constraint.
                //CheckConsistent( (*linkit)->agent, val );   
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
            if( conj->Increment() )
                continue; // Conjecture would like us to try again with new choices
            
            // Conjecture has run out of choices to try.
            return false; 
        }            
        break; // Didn't throw: success
    }    
    
    // Side-info is info required by the Engine that isn't part of the CSP model
    // and would be difficult to re-create from it outside of this class
    // (i.e. only given a set of values for variables).
    if( side_info )
    {
        ASSERT(query); // we should still have the last query that was tried - and matched
        
        // Stolen from AndRuleEngine::CompareLinks()
        FOREACH( shared_ptr<const SR::DecidedQuery::Link> b, *query->GetAbnormalLinks() )
            side_info->abnormal_links.insert( b ); 
        
        FOREACH( shared_ptr<const SR::DecidedQuery::Link> b, *query->GetMultiplicityLinks() )
            side_info->multiplicity_links.insert( b ); 
            
        // Stolen from AndRuleEngine::DecidedCompare()
        if( query->GetEvaluator() )
            side_info->evaluator_records.insert( make_pair( query->GetEvaluator(), *query->GetAbnormalLinks() ) );
    }
    
    return true;
}









#if 0
// Some domain extension code, see #120

        list< TreePtr<Node> > x_to_add;
        Conjecture lconj;
        set<SR::Agent *> lagents;
        lagents.insert( agent ); // just the one agent this time
        lconj.Configure(lagents, agent);
        for( TreePtr<Node> x : x_nodes )
        {
            lconj.Start();
            while(1)
            {
                try
                {
                    query = lconj.GetQuery(agent);
                    agent->RunDecidedQuery( *query, x );
                    FOREACH( shared_ptr<const DecidedQuery::Link> b, *query->GetNormalLinks() )
                    x_to_add.push_back(b->x);
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