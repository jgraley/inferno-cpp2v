
// More than one SC_THREAD in a class

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    sc_event proceed;
    int x;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T1);        
        SC_THREAD(T2);        
    }
    void T1()
    {
        x = 33;
        proceed.notify(SC_ZERO_TIME);
    }
    void T2()
    {
        wait( proceed );
        cease( x + 22 );
    }
};

TopLevel top_level("top_level");

