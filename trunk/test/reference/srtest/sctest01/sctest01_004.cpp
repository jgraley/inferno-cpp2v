#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
int gvar;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
void T();
sc_event proceed;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
void T();
sc_event instigate;
sc_event proceed;
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
add_inst("add_inst"),
mul_inst("mul_inst")
{
SC_THREAD(T);
}
void T();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");

void Adder::T()
{
wait(  ::Adder::proceed );
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
}

void Multiplier::T()
{
wait(  ::Multiplier::instigate );
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
cease(  ::gvar );
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
}
