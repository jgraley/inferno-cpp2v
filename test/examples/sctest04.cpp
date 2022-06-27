// Loop containing a yield in the body, should be rotated to optimise

#include <isystemc.h>

int gvar;
int i;

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
        // #576 put crazy number 29258 where it should be 5 to pass, to test if we can 
        // match on a SpecificInteger with particular value. If all SpecificInteger 
        // are matched, the 0 will become 5 too and we will fail.
#ifdef EXPECTATION_RUN
        int t = 5;
#else
        int t = 29258;
#endif        
        for( i=0; i<t; i++ )
        {
            gvar += i;
            wait(SC_ZERO_TIME);      
            gvar *= 2;
        }
        cease( gvar );
    }
};

TopLevel top_level("top_level");

