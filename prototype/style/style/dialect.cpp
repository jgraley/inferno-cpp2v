
#include "dialect.hpp"
#include <assert.h>

ConstructSet MakeSet( const Construct a[], int l )
{
    ConstructSet s;
    for( int i=0; i<l; i++ )
        s.set( a[i] );
    return s;
}

Dialect Dialect::Universal()
{     
    return ~Dialect();
}

Dialect Dialect::LegacyLLVM()
{
    static const Construct lv[] = { LOCAL_VARIABLE, COND_BRANCH, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP };
    return MAKE_SET( lv );
}

Dialect Dialect::ANSIC()
{
    static const Construct lv[] = { LOCAL_VARIABLE, NESTED_IF, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP };
    return MAKE_SET( lv );
}

Dialect Dialect::CPP()
{
    static const Construct lv[] = { LOCAL_VARIABLE, NESTED_IF, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS };
    return MAKE_SET( lv );
}

bool Dialect::CanApply( const Dialect::Transformation &trans )
{
    // We can apply the transformation when the current dialect is within the allowed dialect
    return *this <= trans.allowed; // alternative: !(*this & ~trans.allowed);
}

void Dialect::Apply( const Transformation &trans )
{
    // You must check the transformation is allowed first - it can't work correctly
    // if not allwoed.
    assert( CanApply( trans ) );
    
    // Add the introduces and remove the eliminates
    *this |= trans.introduces;
    *this &= ~trans.eliminates;
}

Dialect::Transformation::Transformation( const ConstructSet &a, 
                                         const ConstructSet &i, 
                                         const ConstructSet &e,
                                         const std::string n ) :
    allowed(a),
    introduces(i),
    eliminates(e),
    name(n)
{
    // Union of these should be empty; each construct must either be introcduced or eliminated
    // or neither (in which case the input dialect propogates to the output).    
    assert( !(introduces & eliminates).any() );      
    
    // A pass will only be scheduled based on the 
    // constructs it eliminates (thereby helping to move the current_dialect dialect toward the target 
    // dialect). OTherwise the pass will never get run - surely a mistake!
    assert( eliminates.any() );
    
    // We can never be run if we don't allow any constructs at all!
    assert( allowed.any() );
} 

const ConstructSet &Dialect::Transformation::GetAllowed() const
{
    return allowed;
}

const ConstructSet &Dialect::Transformation::GetIntroduces() const
{
    return introduces;
}

const ConstructSet &Dialect::Transformation::GetEliminates() const
{
    return eliminates;
}

static std::vector<Dialect::Transformation> available_passes;
void DefinePasses()
{
    // common subexpression elimination
    {
        static const Construct allow[] = {LOCAL_VARIABLE, NESTED_IF, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS};
        static const Construct elim[] = {EXPRESSION_UNOPT_GCSE};
        Dialect::Transformation d( MAKE_SET( allow ),
                                   ConstructSet(),
                                   MAKE_SET( elim ),
                                   "global common subexpression elimination optimisation" );
        available_passes.push_back(d);                          
    }
    // loop induction variables
    {
        static const Construct allow[] = {LOCAL_VARIABLE, NESTED_IF, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS};
        static const Construct intro[] = {LOCAL_VARIABLE, EXPRESSION_UNOPT_GCSE};
        static const Construct elim[] = {EXPRESSION_UNOPT_LOOP};
        Dialect::Transformation d( MAKE_SET( allow ),
                                   MAKE_SET( intro ),
                                   MAKE_SET( elim ),
                                   "loop induction optimisation" );
        available_passes.push_back(d);                          
    }
    // if lowering
    {
        static const Construct allow[] = {LOCAL_VARIABLE, NESTED_IF, COND_BRANCH, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS};
        static const Construct intro[] = {COND_BRANCH, LOCAL_VARIABLE};
        static const Construct elim[] = {NESTED_IF};
        Dialect::Transformation d( MAKE_SET( allow ),
                                   MAKE_SET( intro ),
                                   MAKE_SET( elim ),
                                   "nested if lowering" );
        available_passes.push_back(d);                          
    }
    // staticise locals
    {
        static const Construct allow[] = {LOCAL_VARIABLE, NESTED_IF, COND_BRANCH, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS};
        static const Construct elim[] = {LOCAL_VARIABLE};
        Dialect::Transformation d( MAKE_SET( allow ),
                                   ConstructSet(),
                                   MAKE_SET( elim ),
                                   "staticising local variables" );
        available_passes.push_back(d);                          
    }
    // modularise classes
    {
        static const Construct allow[] = {NESTED_IF, COND_BRANCH, EXPRESSION_UNOPT_GCSE, EXPRESSION_UNOPT_LOOP, CPP_CLASS};
        static const Construct intro[] = {NESTED_IF};
        static const Construct elim[] = {CPP_CLASS};
        Dialect::Transformation d( MAKE_SET( allow ),
                                   MAKE_SET( intro ),
                                   MAKE_SET( elim ),
                                   "convert classes to SC modules" );
        available_passes.push_back(d);                          
    }                                              
}


typedef std::vector<Dialect::Transformation>::const_iterator PassIt;
bool Schedule( const std::vector<Dialect::Transformation> &available_passes, // vector containing all the passes available
               std::vector<PassIt> &pass_sequence, // we will put iterators to the required passes in here, in order
               std::vector<Dialect> &intermediate_dialects, // we will store the corresponding intermediate dialects here
               Dialect starting_dialect, // input program is in this dialect
               Dialect ending_dialect, // output program is in this dialect (or a subset)
               unsigned depth = 0 ) // recursion depth
{
    ConstructSet want_eliminate = starting_dialect & ~ending_dialect;
    bool found=false;
    unsigned best_size = (unsigned)-1; // large positive number
    PassIt best_pass;
    
    // Iterate through all the passes available to choose from
    for( PassIt pass=available_passes.begin(); pass!=available_passes.end(); pass++ )
    {
        // Throw out passes that would be illegal to apply at this stage
        if( !starting_dialect.CanApply( *pass ) )
            continue;
            
        // Throw out passes that have no benefit because they don't eliminate
        // any of the constructs that are present and that we want to remove.   
        if( (pass->GetEliminates() & want_eliminate).none() )
            continue;    
            
        // Try the current transformation    
        Dialect current_dialect = starting_dialect;
        current_dialect.Apply( *pass );
        
        // If the transformation reached the target dialect
        // (or subset, which is fine) we're done and it took one pass.
        if( current_dialect <= ending_dialect )
        {
            best_size = 1;
            best_pass = pass;
            found = true;
            break; // don't try any more, we're done
        }
        
        // Now recurse, starting from the dialect that resulted from the pass
        // we just applied. If it fails, throw out this pass.
        bool ok = Schedule( available_passes, pass_sequence, intermediate_dialects, current_dialect, ending_dialect, depth+1 );
        if( !ok )
            continue;
            
        // determine whether     
        if( pass_sequence.size()-depth < best_size-1 )
        {
            best_size = pass_sequence.size()+1-depth;
            best_pass = pass;
            found = true;
        }       
    }
    
    // If we failed, don't bother setting up outputs 
    if( !found )
        return false;
       
    // Place the iterator for the chosen pass into the result vector
    pass_sequence.resize( depth+1 );
    pass_sequence[depth] = best_pass; 
    
    // Re-apply the current pass to get the resulting intermediate, and
    // place that in intermediates vector.
    // Note: could just store in eg best_intermediate_dialect
    Dialect current_dialect = starting_dialect;
    current_dialect.Apply( *best_pass );
    intermediate_dialects.resize( depth+1 );
    intermediate_dialects[depth] = current_dialect;

    // re-run the best pass since its passes may have been overwritten 
    // by later iterations of the loop. Don't need this if we reached the
    // end.
    if( best_size > 1 )
        Schedule( available_passes, pass_sequence, intermediate_dialects, intermediate_dialects[depth], ending_dialect, depth+1 );

    return true;
}



               
void Demo()
{
    DefinePasses();
    
    Dialect start = Dialect::CPP();
    
    static const Construct e[] = {COND_BRANCH};
    Dialect end = MAKE_SET( e );
    
    std::vector<PassIt> pass_sequence;
    std::vector<Dialect> intermediate_dialects;
    bool ok = Schedule( available_passes, pass_sequence, intermediate_dialects, start, end );
    
    assert(0);
}               
               