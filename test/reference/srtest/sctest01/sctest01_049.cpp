#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T2);
}
void T2();
sc_event proceed1;
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
int gvar;
TopLevel top_level("top_level");

void Adder::T2()
{
auto void *state1;
wait(  ::Adder::proceed1 );
{
state1=(&&YIELD3);
goto *(state1);
}
YIELD3:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed1 );
{
state1=(&&YIELD4);
goto *(state1);
}
YIELD4:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T1()
{
auto void *state;
wait(  ::Multiplier::instigate );
{
state=(&&YIELD);
goto *(state);
}
YIELD:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state=(&&YIELD1);
goto *(state);
}
YIELD1:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state=(&&YIELD2);
goto *(state);
}
YIELD2:;
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
