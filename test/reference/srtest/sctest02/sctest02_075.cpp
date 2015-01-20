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
bool instigate;
private:
unsigned int state;
public:
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_NEXT_1 = 2U,
T_STATE_PROCEED_THEN_ELSE_1 = 1U,
T_STATE_PROCEED_THEN_ELSE_2 = 3U,
T_STATE_PROCEED_NEXT_2 = 4U,
};
bool proceed;
};
int gvar;
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
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE_1 = 1U,
T_STATE_PROCEED_NEXT_1 = 0U,
};
private:
unsigned int state;
public:
bool proceed;
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
void T();
enum TStates
{
};
 ::Adder add_inst;
};
TopLevel top_level("top_level");

void Multiplier::T()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1 :  ::Multiplier::T_STATE_PROCEED_NEXT);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT== ::Multiplier::state )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1== ::Multiplier::state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2 :  ::Multiplier::T_STATE_PROCEED_NEXT_1);
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT_1 )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT_1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2== ::Multiplier::state )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT_2);
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT_2 )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT_2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
return ;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE )
{
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}
next_trigger(SC_ZERO_TIME);
}

void Adder::T()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE_1 :  ::Adder::T_STATE_PROCEED_NEXT_1);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT_1 )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT_1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE_1);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE_1 )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
