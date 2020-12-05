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
SC_METHOD(T1);
}
enum TStates1
{
T_STATE_PROCEED_NEXT2 = 0U,
T_STATE_PROCEED_NEXT3 = 2U,
T_STATE_PROCEED_NEXT4 = 4U,
T_STATE_PROCEED_THEN_ELSE2 = 1U,
T_STATE_PROCEED_THEN_ELSE3 = 3U,
T_STATE_PROCEED_THEN_ELSE4 = 5U,
};
void T1();
bool instigate;
bool proceed1;
private:
unsigned int state1;
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
add_inst("add_inst"),
mul_inst("mul_inst")
{
SC_THREAD(T2);
}
enum TStates2
{
};
void T2();
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
if( enabled )
{
if(  ::Adder::T_STATE_PROCEED_NEXT== ::Adder::state )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
enabled=(false);
}
}
if( enabled )
{
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE== ::Adder::state )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT1);
}
}
if( enabled )
{
if(  ::Adder::T_STATE_PROCEED_NEXT1== ::Adder::state )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
enabled=(false);
}
}
if( enabled )
{
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE1== ::Adder::state )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
enabled=(false);
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}

void Multiplier::T1()
{
/*temp*/ bool enabled1 = true;
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state1=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
enabled1=(false);
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_NEXT2== ::Multiplier::state1 )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state1=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
enabled1=(false);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2== ::Multiplier::state1 )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state1=((!(! ::Multiplier::proceed1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3 :  ::Multiplier::T_STATE_PROCEED_NEXT3);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_NEXT3== ::Multiplier::state1 )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state1=((! ::Multiplier::proceed1) ?  ::Multiplier::T_STATE_PROCEED_NEXT3 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3);
enabled1=(false);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3== ::Multiplier::state1 )
{
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state1=((!(! ::Multiplier::proceed1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4 :  ::Multiplier::T_STATE_PROCEED_NEXT4);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_NEXT4== ::Multiplier::state1 )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state1=((! ::Multiplier::proceed1) ?  ::Multiplier::T_STATE_PROCEED_NEXT4 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4);
enabled1=(false);
}
}
if( enabled1 )
{
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4== ::Multiplier::state1 )
{
 ::Multiplier::proceed1=(false);
cease(  ::gvar );
enabled1=(false);
}
}
if( enabled1 )
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T2()
{
/*temp*/ bool enabled2 = true;
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
enabled2=(false);
}
