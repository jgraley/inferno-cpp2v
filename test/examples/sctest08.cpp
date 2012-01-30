
// thread with subordinate function

#include <isystemc.h>


//sc_signal<int> gvar, i, j;
int gvar, i, j;

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        //sc_trace(GetSCTraceFP(), gvar, "gvar");
       // sc_trace(GetSCTraceFP(), i, "i");
       // sc_trace(GetSCTraceFP(), j, "j");
        SC_THREAD(T);        
    }
    void T()
    {
        gvar = 1;
        for( i=0; i<4; i=i+1 )
        {
            gvar = gvar + i;
            //gvar = helper(3, gvar);
            helper();
            gvar = gvar * 2;
            wait(SC_ZERO_TIME);      // this yield protects the 0-iteration case of the for-loop, so no inferred yields 
        }      
        cease( gvar );
    }
   /* int helper( int its, int var )
    {
        for( j=0; j<its; j++ )
        {                        
            wait(SC_ZERO_TIME);      
            var++;
        }    
        return var;
    }*/
    void helper()
    {
        for( j=0; j<3; j=j+1 )
        {                        
            wait(SC_ZERO_TIME);      
            gvar=gvar+1;
        }    
        otherhelper();
        return;
    }
    void otherhelper()
    {
        gvar = gvar - 1;
    }
};

TopLevel top_level("top_level");

