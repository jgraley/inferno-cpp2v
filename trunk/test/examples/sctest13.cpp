
// subordinate funciton with multiple parameters and return value

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    int x, i;
    float fi;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);        
    }
    void T()
    {
        x = 0;
        for( i=0; i<4; i++ )
            x += i;
        for( i=0; i<=4; i++ )
            x += i;
        for( i=0; i!=4; i++ )
            x += i;
        for( i=4; i>0; i-- )
            x += i;
        for( i=4; i>=0; i-- )
            x += i;
        for( i=0; i<4; i++ )
            break;
        for( i=0; i<4; i++ )
            switch(0) { case 0: break; }
        for( i=0; i<4; i++ )
            continue;
        for( ; i<4; i++ )
            x += i;
        for( i=0; i<x; i++ )
            {};
        for( i=0; i<4; i=i+1 )
            x += i;
        for( i=0; i<4; i+=1 )
            x += i;
        for( i=4; i>0; i=i-1 )
            x += i;
        for( i=4; i>0; i-=1 )
            x += i;
        for( i=0; i<4; i=4 )
            x += i;
        for( i=0; i<4; i++ )
            i += 0;
        for( i=0; i<4; i++ )
            x += f();
        cease(x);
    }
    int f()
    {
        return 3;
    }
};

TopLevel top_level("top_level");

