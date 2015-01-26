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
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD1 = 0U,
};
sc_event proceed;
};
int gvar;
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
sc_event instigate;
sc_event proceed;
void T();
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
 ::Adder add_inst;
enum TStates
{
};
 ::Multiplier mul_inst;
void T();
};
TopLevel top_level("top_level");

void Adder::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD };
auto unsigned int state;
YIELD:;
if( (0U)==(sc_delta_count()) )
{
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD1;
goto *(lmap[state]);
}
if( state== ::Adder::T_STATE_YIELD1 )
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
YIELD:;
if( (0U)==(sc_delta_count()) )
{
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD;
goto *(lmap[state]);
}
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
