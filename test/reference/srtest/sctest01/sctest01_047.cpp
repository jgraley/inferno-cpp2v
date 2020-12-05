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
void T();
sc_event proceed;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T1);
}
void T1();
sc_event instigate;
sc_event proceed1;
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
add_inst("add_inst"),
mul_inst("mul_inst")
{
SC_THREAD(T2);
}
void T2();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
{
wait(  ::Adder::proceed );
goto YIELD;
YIELD:;
}
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
{
wait(  ::Adder::proceed );
goto YIELD1;
YIELD1:;
}
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T1()
{
{
wait(  ::Multiplier::instigate );
goto YIELD2;
YIELD2:;
}
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
{
wait(  ::Multiplier::proceed1 );
goto YIELD3;
YIELD3:;
}
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
{
wait(  ::Multiplier::proceed1 );
goto YIELD4;
YIELD4:;
}
cease(  ::gvar );
return ;
}

void TopLevel::T2()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
