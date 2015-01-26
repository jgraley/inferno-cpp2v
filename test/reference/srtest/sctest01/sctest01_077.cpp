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
T_STATE_YIELD = 1U,
T_STATE_YIELD1 = 0U,
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
SC_METHOD(T);
}
private:
unsigned int state;
public:
void T();
sc_event instigate;
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 2U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 0U,
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
enum TStates
{
};
void T();
 ::Multiplier mul_inst;
 ::Adder add_inst;
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD1;
enabled=(false);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_YIELD1) )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
enabled=(false);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_YIELD) )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
enabled=(false);
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}

void Multiplier::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD2;
enabled=(false);
}
if( enabled&&( ::Multiplier::T_STATE_YIELD2== ::Multiplier::state) )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD1;
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_YIELD1) )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_YIELD) )
{
cease(  ::gvar );
enabled=(false);
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T()
{
/*temp*/ bool enabled = true;
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
enabled=(false);
}
