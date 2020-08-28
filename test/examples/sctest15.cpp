
// 2 threads with subordinate functions

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
    }
};

TopLevel top_level("top_level");

