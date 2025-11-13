
// 1 thread with subordinate functions

#include <isystemc.h>

class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(U);
    }
    void U()
    {
        HelperU();
    }
    void HelperU()
    { 
		(void)sc_delta_count();        
    } 
};

TopLevel top_level("top_level");

