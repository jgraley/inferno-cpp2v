

#include "isystemc.h"

class myint : public sc_interface
{
};

class mymod : public sc_module, public myint
{ 
    sc_event e;
    void f()
    {
        wait(e);
    }    
};
