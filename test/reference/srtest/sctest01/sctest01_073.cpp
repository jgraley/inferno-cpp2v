#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
int gvar;
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
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
void T();
private:
unsigned int state;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
sc_event instigate;
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 2U,
};
void T();
private:
unsigned int state;
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
void T();
enum TStates
{
};
};
TopLevel top_level("top_level");

void Adder::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
wait(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
continue;
}
if(  ::Adder::state== ::Adder::T_STATE_YIELD )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD1;
continue;
}
if(  ::Adder::state== ::Adder::T_STATE_YIELD1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void Multiplier::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
wait(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
continue;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD1;
continue;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD2;
continue;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD2 )
{
cease(  ::gvar );
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