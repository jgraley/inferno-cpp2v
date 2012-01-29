
#include <isystemc.h>

int gvar;

class Adder : public sc_module
{
public:
    bool proceed;
    SC_CTOR(Adder)
    {
        SC_THREAD(T);
    }
    void T();
};

class Multiplier : public sc_module
{
public:
    bool instigate;
    bool proceed;
    //sc_event yield;
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
        mul_inst.instigate = true;      
    }
};

TopLevel top_level("top_level");

void Adder::T()
{
    while(!proceed)
        wait( SC_ZERO_TIME );
    proceed = false; // reset for next time (should be atomic with the test)
    gvar += 2;
    top_level.mul_inst.proceed = true;
    while(!proceed)
        wait( SC_ZERO_TIME );
    proceed = false; // reset for next time (should be atomic with the test)
    gvar += 3;
    top_level.mul_inst.proceed = true;
}


void Multiplier::T()
{
    while(!instigate)
        wait( SC_ZERO_TIME );
    instigate = false; // reset for next time (should be atomic with the test)
    gvar *= 5;
    top_level.add_inst.proceed = true;
    while(!proceed)
        wait( SC_ZERO_TIME );
    proceed = false; // reset for next time (should be atomic with the test)
    gvar *= 5;    
    top_level.add_inst.proceed = true;
    while(!proceed)
        wait( SC_ZERO_TIME );
    proceed = false; // reset for next time (should be atomic with the test)
    cease(gvar);
}



