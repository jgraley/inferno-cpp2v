// Loop containing another loop which contains a yield, should be rotated to optimise
// Inner loop becomes an if that does not contain the yield, harmless

// Currently, this may not work ideally if the inner loop gets rotated first then 
// the outer, seems to infer yields. This is because rotating loops in general infers ifs
// which can cross yields. Doing this to the inner loop recreates sctest05.cpp, and
// a new non-yielding loop is inferred.
//
// Thus it is correct to do outer loops first, or only. We should never rotate an inner 
// loop before rotating its containing loop because the ifs we infer will become
// loops themselves and infer yields.
// 
// We can't not rotate inner loops at all because consider:
// loop( ...loop( ...yield...) ...yield... )
// Wherein the inner loop needs to be rotated due to a *seperate* yield. Here it is 
// necessary to rotate the outer loop first, then inner.

// NOW FIXED, comment kept for posterity


#include <isystemc.h>

int gvar;
int i, j;

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);
    }
    void T()
    {
        gvar = 1;
        for( i=0; i<4; i++ )
        {
            gvar += i;
            for( j=0; j<3; j++ ) // inferno assumes this could have 0 iterations. If this happens a yield will be inferred
            {                        
                wait(SC_ZERO_TIME);      
                gvar++;
            }
            gvar *= 2;
        }
        exit( gvar );
    }
};

TopLevel top_level("top_level");

