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
        for( i=0; i<5; i++ )
        {
            gvar += i;
            wait(SC_ZERO_TIME);      
            gvar *= 2;
        }
        cease( gvar );
    }
};

TopLevel top_level("top_level");

