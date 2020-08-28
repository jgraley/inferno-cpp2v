#include "systemic_constraint.hpp"
#include "query.hpp"
#include "agent.hpp"

using namespace SR;

SystemicConstraint::SystemicConstraint( Agent *agent_ ) :
    agent(agent_),
    pq( agent->GetPatternQuery() ),
    conj( make_shared<Conjecture>() )
{    
    Set<Agent *> my_agents;
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


bool SystemicConstraint::Test( list< TreePtr<Node> > values )
{
    ASSERT( values.size() == GetDegree() );
    
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
            shared_ptr<DecidedQuery> query = conj->GetQuery(agent);
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
                    throw Mismatch();  
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
    return true;
}
