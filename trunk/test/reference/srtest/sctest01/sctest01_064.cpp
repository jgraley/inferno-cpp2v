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
sc_event instigate;
void T();
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD_1 = 1U,
T_STATE_YIELD_2 = 2U,
};
sc_event proceed;
};
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD_1 = 0U,
};
void T();
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
enum TStates
{
};
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
int gvar;

void Multiplier::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&YIELD_2, &&YIELD_1, &&YIELD };
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD;
YIELD:;
YIELD_1:;
YIELD_2:;
if(  ::Multiplier::T_STATE_YIELD==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_1;
goto *(lmap[state]);
}
if(  ::Multiplier::T_STATE_YIELD_1==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD_2;
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_YIELD_2 )
{
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}

void Adder::T()
{
static const unsigned int (lmap[]) = { &&YIELD_1, &&YIELD };
auto unsigned int state;
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD_1;
YIELD:;
YIELD_1:;
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

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
