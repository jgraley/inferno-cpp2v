
#include <isystemc.h>

class Adder : public sc_module
{
public:
    SC_CTOR(Adder)
    {
    }
};

class TopLevel : public sc_module
{
public:
    Adder add_inst;
    SC_CTOR(TopLevel) :
        add_inst("add_inst")        
    {
    }
};

TopLevel top_level("top_level");
