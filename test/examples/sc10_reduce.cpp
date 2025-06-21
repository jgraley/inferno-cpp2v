
// subordinate function with multiple parameters and return value

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
        cease( f(1) );
    }
    int f( int i )
    {
        return i;
    }
};

TopLevel top_level("top_level");

