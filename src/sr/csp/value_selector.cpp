#include "reference_solver.hpp"
#include "solver_holder.hpp"
#include "query.hpp"
#include "agents/agent.hpp"
#include "the_knowledge.hpp"
#include "../sym/result.hpp"

#include <algorithm>

// Gather stats on the successiness of simple forward checking
#define GATHER_GSV

// Define this to check we don't get as far as trying to make a values generator
// for an empty set (eg because the residual of the domain after forward-checking 
// was empty).
//#define CHECK_NONEMPTY_RESIDUAL

using namespace CSP;

ValueSelector::ValueSelector( const ConstraintSet &constraints_to_query_, 
                              const SR::TheKnowledge *knowledge_,
                              Assignments &assignments_,
                              VariableId var ) :
    knowledge( knowledge_ ),
    assignments( assignments_ ),
    my_var( var ),
    constraints_to_query( constraints_to_query_ )
{
    INDENT("V");
       
    list<shared_ptr<SYM::SymbolSetResult>> rl; 
    for( shared_ptr<Constraint> c : constraints_to_query )
    {                               
        shared_ptr<SYM::SymbolSetResult> r = c->GetSuggestedValues( assignments, my_var );
        ASSERT( r );
        rl.push_back(r);
    }

    auto s = make_shared<set<Value>>(); // could be unique_ptr in C++14 when we can move-capture
    shared_ptr<SYM::SymbolSetResult> result = SYM::SymbolSetResult::GetIntersection(rl);
    ASSERT( result );
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
        SetupSuggestionGenerator( s );
    else
        SetupDefaultGenerator();
}

       
ValueSelector::~ValueSelector()
{
}


void ValueSelector::SetupDefaultGenerator()
{
    SR::TheKnowledge::DepthFirstOrderedIt fwd_it = knowledge->depth_first_ordered_domain.begin();     
    values_generator = [=]() mutable -> Value
    {
        if( fwd_it==knowledge->depth_first_ordered_domain.end() )        
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
    TRACE("At ")(my_var)(", got suggestion ")(*suggested)(" - rewriting queue\n"); 
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

