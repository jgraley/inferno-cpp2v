
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
        x = 4;
        ++x && (y = f(x));
        !(++x) || (y += f(x));
        for( x=0; x<2; )
            x++ ? (y+=f(x)) : (y-=f(x));
        cease( y );
    }
    int f( int i )
    {
        return 100/i;
    }
};

TopLevel top_level("top_level");

