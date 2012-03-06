
// subordinate funciton with multiple parameters and return value

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    int x, y;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);        
    }
    void T()
    {
        x = 0;
        ++x && (y = f(x));
        cease( y );
    }
    int f( int i )
    {
        return i+9;
    }
};

TopLevel top_level("top_level");

