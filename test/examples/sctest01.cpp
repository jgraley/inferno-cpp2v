
#include <isystemc.h>

int gvar;

class Adder : public sc_module
{
public:
    sc_event proceed;
    SC_CTOR(Adder)
    {
        SC_THREAD(T);
    }
    void T();
};

class Multiplier : public sc_module
{
public:
    sc_event instigate;
    sc_event proceed;
    SC_CTOR(Multiplier) 
    {
        SC_THREAD(T);
    }
    void T();
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
        SC_THREAD(T);
    }
    void T()
    {
        gvar = 1;
        mul_inst.instigate.notify();      
    }
};

TopLevel top_level("top_level");

void Adder::T()
{
    //wait( proceed );
    gvar += 2;
    top_level.mul_inst.proceed.notify();
    //wait( proceed );
    gvar += 3;
    top_level.mul_inst.proceed.notify();
}


void Multiplier::T()
{
    wait( instigate );
    gvar *= 5;
    top_level.add_inst.proceed.notify();
    //wait( proceed );
    gvar *= 5;
    top_level.add_inst.proceed.notify();
    //wait( proceed );
    exit(gvar);
}


