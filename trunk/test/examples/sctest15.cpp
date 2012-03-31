
// 2 threads with subordinate functions

#include <isystemc.h>

int gvar=0;

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);
        SC_THREAD(U);
    }
    void T()
    {
        HelperT();
        HelperT();
        HelperT();
        exit(gvar);
    }
    void HelperT()
    {
        gvar++;
        if( gvar<5 )
            HelperT();
    }
    void U()
    {
        HelperU();
    }
    void HelperU()
    {
    }
};

TopLevel top_level("top_level");

