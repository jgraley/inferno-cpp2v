#include "systemic_constraint.hpp"
#include "query.hpp"
#include "agent.hpp"

using namespace CSP;

SystemicConstraint::SystemicConstraint( SR::Agent *agent_ ) :
    agent(agent_),
    pq( agent->GetPatternQuery() ),
    conj( make_shared<SR::Conjecture>() )
{    
    Set<SR::Agent *> my_agents;
    my_agents.insert( agent ); // just the one agent this time
    conj->Configure(my_agents, agent_);
}


int SystemicConstraint::GetDegree() const
{
    // There's a variable for each pattern node. Count the number 
    // potentially constrained by our agent. Exclude abnormal links 
    // and multiplicities - we only want ones that will be part of our 
    // AndRule region.
    return pq->GetNormalLinks()->size() + 
           1; // plus one for the location of our x (constaints are non-localised)
}


list<VariableId> SystemicConstraint::GetVariables() const
{
    list<VariableId> vars;
    
    vars.push_back( agent ); // Our agent is one of them!
    
    FOREACH( const SR::PatternQuery::Link &b, *pq->GetNormalLinks() )
        vars.push_back( b.agent );  // This rest are our normal linked agents
        
    return vars;
}


bool SystemicConstraint::Test( list< Value > values,
                               SideInfo *side_info )
{
    ASSERT( values.size() == GetDegree() );
    for( Value v : values )
    {
        ASSERT( v != NullValue );
    }
    // We're going to be lazy and borrow the Conjecture class for now.
    // Our constructor has already configured it to have our agent and
    // only our agent, so we won't really be using the multi-agent support
    // the Conjecture offers. We really just want the algorithm out of
    // Conjecture::IncrementAgent().
    //
    // Prepare the conjecture for a series of iterations.
    conj->Start();
    
    // "our" X is simply the first value (i.e. first degree of freedom
    // is being moved around the X tree).
    TreePtr<Node> x = values.front();
    values.pop_front();
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
            // Similar to AndRuleEngine::DecidedCompare(), we get the
            // Query object from conjecture, and run a query on it.
            query = conj->GetQuery(agent);
            agent->RunDecidedQuery( *query, x );

            // The query now has populated links, which should be full
            // (otherwise RunDecidedQuery() (DQ) should have thrown). We loop 
            // over both and check that they refer to the same x nodes
            // we were passed. Mismatch will throw, same as in DQ.
            auto nlinks = query->GetNormalLinks();
            ASSERT( nlinks->size() == values.size() );
            auto nit = nlinks->begin();            
            for( TreePtr<Node> child_x : values )
            {
                if( child_x != nit->x )
                    throw NormalLinkMismatch();  
                nit++;
            }
        }
        catch( ::Mismatch & )
        {
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
    
    // Side-info is info required by the Engins that isn't part of the CSP model
    // and would be difficult to re-create from it outside of this class
    // (i.e. only given a set of values for variables).
    if( side_info )
    {
        ASSERT(query); // we should still have the last query that was tried - and matched
        
        // Stolen from AndRuleEngine::CompareLinks()
        FOREACH( const SR::DecidedQuery::Link &b, *query->GetAbnormalLinks() )
            side_info->abnormal_links.insert( make_pair(query, &b) ); 
        
        FOREACH( const SR::DecidedQuery::Link &b, *query->GetMultiplicityLinks() )
            side_info->multiplicity_links.insert( make_pair(query, &b) ); 
            
        // Stolen from AndRuleEngine::DecidedCompare()
        if( query->GetEvaluator() )
            side_info->evaluator_queries.insert(query);
    }
    
    return true;
}