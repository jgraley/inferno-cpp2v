
// nested loops with 2 yields, at different levels

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
            for( j=0; j<3; j++ )
            {                        
                wait(SC_ZERO_TIME);      
                gvar++;
            }
            gvar *= 2;
            wait(SC_ZERO_TIME);      // this yield protects the 0-iteration case of the for-loop, so no inferred yields 
        }
        cease( gvar );
    }
};

TopLevel top_level("top_level");

