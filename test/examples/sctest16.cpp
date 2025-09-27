
#include <isystemc.h>

int gvar;

class Adder : public sc_module
{
public:
    SC_CTOR(Adder)
    {
    }
};

class Multiplier : public sc_module
{
public:
    SC_CTOR(Multiplier) 
    {
    }
};

class TopLevel : public sc_module
{
public:
    Adder add_inst;
    Multiplier mul_inst;
    SC_CTOR(TopLevel) :
        add_inst("add_inst"),
        mul_inst("mul_inst")        
    {
    }
};

TopLevel top_level("top_level");

