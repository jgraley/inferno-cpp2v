#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_METHOD(T2);
}
enum TStates2
{
T_STATE_YIELD3 = 0U,
T_STATE_YIELD4 = 1U,
};
void T2();
sc_event proceed1;
private:
unsigned int state1;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_METHOD(T1);
}
enum TStates1
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 2U,
};
void T1();
sc_event instigate;
sc_event proceed;
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
enum TStates
{
};
void T();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T2()
{
/*temp*/ bool enabled2 = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Adder::proceed1 );
 ::Adder::state1= ::Adder::T_STATE_YIELD3;
enabled2=(false);
}
if( enabled2 )
{
if(  ::Adder::T_STATE_YIELD3== ::Adder::state1 )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed1 );
 ::Adder::state1= ::Adder::T_STATE_YIELD4;
enabled2=(false);
}
}
if( enabled2 )
{
if(  ::Adder::T_STATE_YIELD4== ::Adder::state1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
enabled2=(false);
}
}
if( enabled2 )
next_trigger(SC_ZERO_TIME);
}

void Multiplier::T1()
{
/*temp*/ bool enabled1 = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
enabled1=(false);
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_YIELD== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD1;
enabled1=(false);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_YIELD1== ::Multiplier::state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD2;
enabled1=(false);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_YIELD2== ::Multiplier::state )
{
cease(  ::gvar );
enabled1=(false);
}
}
if( enabled1 )
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T()
{
/*temp*/ bool enabled = true;
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
enabled=(false);
}
