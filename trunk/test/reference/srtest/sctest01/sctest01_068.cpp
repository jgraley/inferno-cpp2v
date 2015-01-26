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
sc_event proceed;
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
T_STATE_YIELD1 = 0U,
T_STATE_YIELD2 = 1U,
};
sc_event instigate;
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
 ::Multiplier mul_inst;
enum TStates
{
};
void T();
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&, && };
do
{
if( (0U)==(sc_delta_count()) )
{
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
continue;
}
if( state== ::Adder::T_STATE_YIELD )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD1;
continue;
}
if( state== ::Adder::T_STATE_YIELD1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
}
while( true );
}

void Multiplier::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&, &&, && };
do
{
if( (0U)==(sc_delta_count()) )
{
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD1;
continue;
}
if( state== ::Multiplier::T_STATE_YIELD1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD2;
continue;
}
if( state== ::Multiplier::T_STATE_YIELD2 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD;
continue;
}
if( state== ::Multiplier::T_STATE_YIELD )
{
cease(  ::gvar );
return ;
}
}
while( true );
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
