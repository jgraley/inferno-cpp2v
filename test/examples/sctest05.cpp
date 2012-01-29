// Loop containing an if which contains a yield, should be rotated to optimise but
// an inner loop that does not yield will be produced.

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
        for( i=0; i<5; i++ )
        {
            gvar += i;
            if( i%2==0 )
            {                        
                wait(SC_ZERO_TIME);      
                gvar ^= 1;
            }
            gvar *= 2;
        }
        cease( gvar );
    }
};

TopLevel top_level("top_level");

