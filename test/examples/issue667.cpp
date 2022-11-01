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
        int a;
        a = 54257; // crazy number, see FixCrazyNumberEmb
        a = 100; // between 54257 and 50
        cease( a );
    }
};

TopLevel top_level("top_level");

