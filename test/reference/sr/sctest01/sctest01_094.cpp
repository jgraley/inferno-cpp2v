#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_METHOD(T);
}
enum TStates
{
T_STATE_YIELD = 0,
T_STATE_YIELD_1 = 1,
};
sc_event proceed;
void T();
private:
unsigned int state;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_METHOD(T_1);
}
enum TStates_1
{
T_STATE_YIELD_2 = 0,
T_STATE_YIELD_3 = 1,
T_STATE_YIELD_4 = 2,
};
sc_event instigate;
sc_event proceed_1;
void T_1();
private:
unsigned int state_1;
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
add_inst("add_inst"),
mul_inst("mul_inst")
{
SC_THREAD(T_2);
}
enum TStates_2
{
};
void T_2();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0) )
{
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
enabled=(false);
}
if( enabled )
{
if(  ::Adder::T_STATE_YIELD== ::Adder::state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD_1;
enabled=(false);
}
}
if( enabled )
{
if(  ::Adder::T_STATE_YIELD_1== ::Adder::state )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
enabled=(false);
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}

void Multiplier::T_1()
{
/*temp*/ bool enabled_1 = true;
if( (sc_delta_count())==(0) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state_1= ::Multiplier::T_STATE_YIELD_2;
enabled_1=(false);
}
if( enabled_1 )
{
if(  ::Multiplier::T_STATE_YIELD_2== ::Multiplier::state_1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed_1 );
 ::Multiplier::state_1= ::Multiplier::T_STATE_YIELD_3;
enabled_1=(false);
}
}
if( enabled_1 )
{
if(  ::Multiplier::T_STATE_YIELD_3== ::Multiplier::state_1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed_1 );
 ::Multiplier::state_1= ::Multiplier::T_STATE_YIELD_4;
enabled_1=(false);
}
}
if( enabled_1 )
{
if(  ::Multiplier::T_STATE_YIELD_4== ::Multiplier::state_1 )
{
cease(  ::gvar );
enabled_1=(false);
}
}
if( enabled_1 )
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T_2()
{
/*temp*/ bool enabled_2 = true;
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
enabled_2=(false);
}
