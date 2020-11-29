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
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT1 = 2U,
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
};
void T();
bool proceed;
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
enum TStates
{
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_NEXT2 = 4U,
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_THEN_ELSE2 = 5U,
};
void T();
bool instigate;
bool proceed;
private:
unsigned int state;
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
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
enabled=(false);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT) )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
enabled=(false);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE) )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT1);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT1) )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
enabled=(false);
}
if( enabled&&( ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE1) )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
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
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT1);
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT1) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE) )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE1) )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT2) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
enabled=(false);
}
if( enabled&&( ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE2) )
{
 ::Multiplier::proceed=(false);
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
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
enabled=(false);
}
