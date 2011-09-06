
// thread with subordinate function

#include <isystemc.h>

//int gvar;
//int i, j;

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);
    }
    void T()
    {
      //  gvar = 1;
      //  for( i=0; i<4; i++ )
      //  {
      //      gvar += i;
            //gvar = helper(3, gvar);
            Helper();
      //      gvar *= 2;
      //      wait(SC_ZERO_TIME);      // this yield protects the 0-iteration case of the for-loop, so no inferred yields 
      //  }
      //  exit( gvar );
    }
/*    int helper( int its, int var )
    {
        for( j=0; j<its; j++ )
        {                        
            wait(SC_ZERO_TIME);      
            var++;
        }    
        return var;
    }*/
    void Helper()
    {
     /*   for( j=0; j<3; j++ )
        {                        
            wait(SC_ZERO_TIME);      
            gvar++;
        }    
        return;*/
    }
};

//TopLevel top_level("top_level");

