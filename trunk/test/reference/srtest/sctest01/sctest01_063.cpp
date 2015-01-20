#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
void T();
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD_1 = 2U,
T_STATE_YIELD_2 = 0U,
};
sc_event instigate;
};
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
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD_1 = 1U,
};
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
void T();
 ::Adder add_inst;
enum TStates
{
};
};
TopLevel top_level("top_level");

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD_1, &&YIELD_2 };
auto unsigned int state;
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD_2;
YIELD:;
if(  ::Multiplier::T_STATE_YIELD_2==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD_1:;
if(  ::Multiplier::T_STATE_YIELD==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_1;
goto *(lmap[state]);
}
YIELD_2:;
if( state== ::Multiplier::T_STATE_YIELD_1 )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}

void Adder::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD_1 };
auto unsigned int state;
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
YIELD:;
if(  ::Adder::T_STATE_YIELD==state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD_1;
goto *(lmap[state]);
}
YIELD_1:;
if( state== ::Adder::T_STATE_YIELD_1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap[state]);
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
