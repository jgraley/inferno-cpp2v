
// subordinate funciton with multiple parameters

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
        f(4, 6, 8);
        cease(x);
    }
    void f( int i, short j, char k )
    {
        x = i+j*3+k*5;
    }
};

TopLevel top_level("top_level");

