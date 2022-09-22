#include "reference_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "db/x_tree_database.hpp"
#include "../sym/result.hpp"

#include <algorithm>

// Gather stats on the successiness of simple forward checking
#define GATHER_GSV

// Define this to check we don't get as far as trying to make a values generator
// for an empty set (eg because the residual of the domain after forward-checking 
// was empty).
//#define CHECK_NONEMPTY_RESIDUAL

#define LOG_INDIVIDUAL_SUGGESTION_SETS

using namespace CSP;

ValueSelector::ValueSelector( const ConstraintSet &constraints_to_query_, 
                              const SR::XTreeDatabase *x_tree_db_,
                              Assignments &assignments_,
                              VariableId var ) :
    x_tree_db( x_tree_db_ ),
    assignments( assignments_ ),
    my_var( var ),
    constraints_to_query( constraints_to_query_ )
{
    INDENT("V");
	TRACE("Making value selector for ")(my_var)("\n"); 		
#ifdef LOG_INDIVIDUAL_SUGGESTION_SETS
    TRACEC("given assignments:\n")(assignments)("\n");
#endif    
    list<unique_ptr<SYM::SetResult>> rl; 
    for( shared_ptr<Constraint> c : constraints_to_query )
    {                        
		TRACEC("Querying ")(c)(" for suggestion set\n");       
        unique_ptr<SYM::SetResult> r = c->GetSuggestedValues( assignments, my_var );
        ASSERT( r );
#ifdef LOG_INDIVIDUAL_SUGGESTION_SETS
        TRACEC("got suggestion ")(r)("\n");
        auto s = make_shared<set<Value>>(); // could be unique_ptr in C++14 when we can move-capture
        bool sok = r->TryGetAsSetOfXLinks(*s);
#endif    
        rl.push_back(move(r));
    }

    unique_ptr<SYM::SetResult> result = SYM::SetResult::GetIntersection(move(rl));
    ASSERT( result );
    auto s = make_shared<set<Value>>(); // could be unique_ptr in C++14 when we can move-capture
    bool sok = result->TryGetAsSetOfXLinks(*s);
       
#ifdef GATHER_GSV
    gsv_n++;
    if( !sok )
        gsv_nfail++;
    else if( s->empty() )
        gsv_nempty++;
    else
        gsv_tot += s->size();
#endif       
              
    if( sok )
    {
		TRACEC("Intersection of suggestions is ")(*s)(" - make queue from this\n"); 		
        SetupSuggestionGenerator( s );
	}
    else
    {
		TRACEC("No suggestion - make default queue\n"); 		
        SetupDefaultGenerator();
	}
}

       
ValueSelector::~ValueSelector()
{
}


void ValueSelector::SetupDefaultGenerator()
{
    SR::DBCommon::DepthFirstOrderedIt fwd_it = x_tree_db->GetIndexes().depth_first_ordered_index.begin();     
    values_generator = [=]() mutable -> Value
    {
        if( fwd_it==x_tree_db->GetIndexes().depth_first_ordered_index.end() )        
            return Value();
        
        Value v = *fwd_it;
        ++fwd_it;
        return v;          
    };
}


void ValueSelector::SetupSuggestionGenerator( shared_ptr<set<Value>> suggested )
{
#ifdef CHECK_NONEMPTY_RESIDUAL
    ASSERT( !suggested->empty() );
#endif
     // Use of shared_ptr here allows the lambda to keep suggested
     // alive without copying it. Even if we could deal with the slowness of a copy, 
     // we'd still get a crash because the initial suggestion_iterator would be
     // invalid for the copy. Could be unique_ptr in C++14 when we can move-capture
    // Taking hint means new generator that only reveals the hint
    set<Value>::iterator suggestion_iterator = suggested->begin();
    values_generator = [=]() mutable -> Value
    {
        if( suggestion_iterator != suggested->end() )
        {                     
            Value v = *suggestion_iterator;
            suggestion_iterator++;
            return v;
        }
        else
        {
            return Value();
        }
    };
}


Value ValueSelector::GetNextValue()
{
    // Use the lambda
    return values_generator();
}


void ValueSelector::DumpGSV()
{
    FTRACES("Suggestions dump\n");
    FTRACEC("Failure to extensionalise: %f%%\n", 100.0*gsv_nfail/gsv_n);
    FTRACEC("Empty set: %f%%\n", 100.0*gsv_nempty/gsv_n);
    FTRACEC("Average size of successful, non-empty: %f\n", 1.0*gsv_tot/(gsv_n-gsv_nfail-gsv_nempty));
}


uint64_t ValueSelector::gsv_n = 0;
uint64_t ValueSelector::gsv_nfail = 0;
uint64_t ValueSelector::gsv_nempty = 0;
uint64_t ValueSelector::gsv_tot = 0;

