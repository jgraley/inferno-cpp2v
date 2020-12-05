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
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
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
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 2U,
};
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
enum TStates
{
};
void T();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
static const unsigned int (lmap[]) = { &&YIELD1, &&YIELD };
auto unsigned int state;
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
YIELD:;
YIELD1:;
if( state== ::Adder::T_STATE_YIELD )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD1;
goto *(lmap[state]);
}
if( state== ::Adder::T_STATE_YIELD1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap[state]);
}

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&YIELD2, &&YIELD1, &&YIELD };
auto unsigned int state;
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD;
YIELD:;
YIELD1:;
YIELD2:;
if( state== ::Multiplier::T_STATE_YIELD )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD1;
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_YIELD1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD2;
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_YIELD2 )
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
