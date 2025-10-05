
// subordinate funciton with multiple parameters and return value

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
        switch(0)
        {
            case 1:
                x=99;
            break;
            case 0:
            if(x==0 || x==2)
                x=false ? 88 : 2;   
            break;
            default:
            break;
        }
        switch(0)
        {
            case 1:
                x=99;
            break;
            case 4:
                x=44; // falling through...
            case 0:
            if(x==0 || x==2)
                x=false ? 88 : 2;   
            break;
        }
        switch(2)
        {
            case 1:
                x=99;                
            break;
            case 2:
            if(x==0 || x==2)
                x=false ? 88 : f();   
            break;
        }
        cease(x);
    }
    int f()
    {
        return 3;
    }
};

TopLevel top_level("top_level");

