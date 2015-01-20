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
enum TStates
{
T_STATE_YIELD = 2U,
T_STATE_YIELD_1 = 1U,
T_STATE_YIELD_2 = 0U,
};
sc_event proceed;
private:
unsigned int state;
public:
void T();
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
private:
unsigned int state;
public:
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD_1 = 1U,
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

void Multiplier::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
wait(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD_2;
continue;
}
if(  ::Multiplier::T_STATE_YIELD_2== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD_1;
continue;
}
if(  ::Multiplier::T_STATE_YIELD_1== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
continue;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD )
{
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void Adder::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
wait(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
continue;
}
if(  ::Adder::T_STATE_YIELD== ::Adder::state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD_1;
continue;
}
if(  ::Adder::state== ::Adder::T_STATE_YIELD_1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
