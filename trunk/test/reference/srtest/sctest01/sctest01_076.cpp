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
void T();
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD1 = 0U,
T_STATE_YIELD2 = 2U,
};
sc_event instigate;
sc_event proceed;
private:
unsigned int state;
};
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_METHOD(T);
}
void T();
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
private:
unsigned int state;
public:
sc_event proceed;
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
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD1;
enabled=(false);
}
if( enabled )
{
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD;
enabled=(false);
}
}
if( enabled )
{
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed );
 ::Multiplier::state= ::Multiplier::T_STATE_YIELD2;
enabled=(false);
}
}
if( enabled )
{
if(  ::Multiplier::state== ::Multiplier::T_STATE_YIELD2 )
{
cease(  ::gvar );
enabled=(false);
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}

void Adder::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
enabled=(false);
}
if( enabled )
{
if(  ::Adder::state== ::Adder::T_STATE_YIELD )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD1;
enabled=(false);
}
}
if( enabled )
{
if(  ::Adder::state== ::Adder::T_STATE_YIELD1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
enabled=(false);
}
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
