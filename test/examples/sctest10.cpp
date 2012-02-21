
// subordinate funciton with multiple parameters and return value

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    int x;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);        
    }
    void T()
    {
        x = 0;
        x = f(4, 6, 8);
        cease(x);
    }
    int f( int i, short j, char k )
    {
        int t = i+j*3;
        return t+k*5;
    }
};

TopLevel top_level("top_level");

