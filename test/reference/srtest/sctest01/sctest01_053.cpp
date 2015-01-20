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
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD_1 = 0U,
};
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
sc_event proceed;
sc_event instigate;
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD_1 = 1U,
T_STATE_YIELD_2 = 2U,
};
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
enum TStates
{
};
 ::Adder add_inst;
void T();
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD_1 };
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD_1;
goto *(lmap[state]);
YIELD:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
goto *(lmap[state]);
YIELD_1:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD_1, &&YIELD_2 };
auto unsigned int state;
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD;
goto *(lmap[state]);
YIELD:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_1;
goto *(lmap[state]);
YIELD_1:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_2;
goto *(lmap[state]);
YIELD_2:;
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
