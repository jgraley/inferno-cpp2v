
// thread with subordinate function, which recurses

#include <isystemc.h>


//sc_signal<int> i, x;
int i, x;

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
       // sc_trace(GetSCTraceFP(), i, "i");
       // sc_trace(GetSCTraceFP(), x, "x");
        SC_THREAD(T);        
    }
    void T()
    {
        i = 0; 
        x = 0;
        recurser();
        cease(x);
    }
    void recurser()
    {
        i++; // i is depth so decrement again at the end
        x++; // x is a total, so just increment
        if( i < 5 )
            recurser();
        i--;
    }
};

TopLevel top_level("top_level");

