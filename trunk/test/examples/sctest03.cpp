// This example demonstrates "clock with no clock" style. We
// use the (documented) yield approach from sctest02, but with an SC_MODULE
// so that we are arranging for it to be invoked repeatedly in different 
// delta cycles. I'm not sure whether this pushes out other processes, but
// if so it won't be any worse than before state-out anyway.


#include <isystemc.h>

int gvar=0;
int tot=0;

class TopLevel : sc_module
{
public:
    SC_CTOR(TopLevel)
    { 
        SC_METHOD(method);
    }
    void method()
    {
        gvar++;
        tot+=gvar;
        if( gvar==10 )
            exit(tot);
        next_trigger( SC_ZERO_TIME );                         
    }
};

TopLevel top_level("top_level");

