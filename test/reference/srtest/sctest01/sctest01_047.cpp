#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
sc_event proceed;
void T();
};
int gvar;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
sc_event proceed;
sc_event instigate;
void T();
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
mul_inst("mul_inst"),
add_inst("add_inst")
{
SC_THREAD(T);
}
 ::Multiplier mul_inst;
 ::Adder add_inst;
void T();
};
TopLevel top_level("top_level");

void Adder::T()
{
{
wait(  ::Adder::proceed );
goto YIELD;
YIELD:;
}
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
{
wait(  ::Adder::proceed );
goto YIELD_1;
YIELD_1:;
}
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T()
{
{
wait(  ::Multiplier::instigate );
goto YIELD;
YIELD:;
}
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
{
wait(  ::Multiplier::proceed );
goto YIELD_1;
YIELD_1:;
}
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
{
wait(  ::Multiplier::proceed );
goto YIELD_2;
YIELD_2:;
}
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
