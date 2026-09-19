
// thread with subordinate function, which recurses

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    int x;
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);        
    }
private:
    void T()
    {
        x = 0;
        recurser(1);
        cease(x);
    }

    void recurser( int i )
    {
        x++; // x is a total, so just increment
        if( i < 5 )
        {
            recurser(i+1);
            recurser(i+1);
        }
    }
};

TopLevel top_level("top_level");

