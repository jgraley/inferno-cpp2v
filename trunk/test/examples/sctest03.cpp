// This example demonstrates "clock with no clock" style. We
// use the (documented) yield approach from sctest02, but with an SC_MODULE
// so that we are arranging for it to be invoked repeatedly in different 
// delta cycles. I'm not sure whether this pushes out other processes, but
// if so it won't be any worse than before state-out anyway.


#include <isystemc.h>

int gvar = 0; 
int tot = 0; 


class TopLevel : sc_module
{
public:
    SC_CTOR(TopLevel)
    { 
        SC_METHOD(method);
    }
    void method()
    {
#ifdef RESET    
        if( sc_delta_count() == 0 ) // The only way I can see to detect that we're in the initialisation phase and getting the
                                    // "free" invocation that happens when you don't call dont_initialize(). This is like a 
                                    // reset, and I'll use it as such until reset wiring goes in. Sadly the value will wrap,
                                    // but happily not until 2^64 delta cycles.
        {
            // Change the values from those set in constructor, to see if reset is working
            gvar = 3;
            tot = -7;
        }
#endif        
        gvar++;
        tot+=gvar;
        if( gvar==10 )
            cease(tot);
        next_trigger( SC_ZERO_TIME );                         
    }
    int gvar;
    int tot;
};

TopLevel top_level("top_level");

