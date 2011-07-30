
#include <isystemc.h>

int gvar;


inline void xwait( bool &x )
{
    while(!x)
        wait( SC_ZERO_TIME );
    x=false; // reset for next time (should be atomic with the test)
}

void xnotify( bool &x )
{
    x=true;
}


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
        xnotify( mul_inst.instigate );      
    }
};

TopLevel top_level("top_level");

void Adder::T()
{
    xwait( proceed );
    gvar += 2;
    xnotify( top_level.mul_inst.proceed );
    xwait( proceed );
    gvar += 3;
    xnotify( top_level.mul_inst.proceed );
}


void Multiplier::T()
{
    xwait( instigate );
    gvar *= 5;
    xnotify( top_level.add_inst.proceed );
    xwait( proceed );
    gvar *= 5;    
    xnotify( top_level.add_inst.proceed );
    xwait( proceed );
    exit(gvar);
}



