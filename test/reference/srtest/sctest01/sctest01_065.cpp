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
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD_1 = 0U,
};
void T();
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 2U,
T_STATE_YIELD_1 = 0U,
T_STATE_YIELD_2 = 1U,
};
void T();
sc_event instigate;
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
enum TStates
{
};
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD };
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD_1;
YIELD:;
if(  ::Adder::T_STATE_YIELD_1==state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::Adder::T_STATE_YIELD )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap[state]);
}

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD, &&YIELD };
auto unsigned int state;
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD_1;
YIELD:;
if(  ::Multiplier::T_STATE_YIELD_1==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_2;
goto *(lmap[state]);
}
if(  ::Multiplier::T_STATE_YIELD_2==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD;
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_YIELD )
{
cease(  ::gvar );
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
