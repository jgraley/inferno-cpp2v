#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_METHOD(T);
}
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 2U,
T_STATE_YIELD_1 = 1U,
T_STATE_YIELD_2 = 0U,
};
private:
unsigned int state;
public:
sc_event instigate;
void T();
};
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_METHOD(T);
}
void T();
private:
unsigned int state;
public:
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
enum TStates
{
};
 ::Adder add_inst;
void T();
};
TopLevel top_level("top_level");
int gvar;

void Multiplier::T()
{
if( (0U)==(sc_delta_count()) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD_2;
return ;
}
if(  ::Multiplier::T_STATE_YIELD_2== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD_1;
return ;
}
if(  ::Multiplier::T_STATE_YIELD_1== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
return ;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD )
{
cease(  ::gvar );
return ;
}
next_trigger(SC_ZERO_TIME);
}

void Adder::T()
{
if( (0U)==(sc_delta_count()) )
{
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
return ;
}
if(  ::Adder::T_STATE_YIELD== ::Adder::state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD_1;
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_YIELD_1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
