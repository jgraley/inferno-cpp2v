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
private:
unsigned int state;
public:
bool proceed;
enum TStates
{
T_STATE_PROCEED_NEXT = 4U,
T_STATE_PROCEED_NEXT1 = 2U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT2 = 0U,
T_STATE_PROCEED_THEN_ELSE2 = 1U,
};
bool instigate;
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
private:
unsigned int state;
public:
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_NEXT1 = 0U,
};
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
void T();
enum TStates
{
};
 ::Multiplier mul_inst;
 ::Adder add_inst;
};
TopLevel top_level("top_level");

void Multiplier::T()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT2== ::Multiplier::state )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2== ::Multiplier::state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT1);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT1== ::Multiplier::state )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
return ;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1== ::Multiplier::state )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT== ::Multiplier::state )
{
next_trigger(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
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
if( (0U)==(sc_delta_count()) )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT1);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT1 )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT);
}
if(  ::Adder::T_STATE_PROCEED_NEXT== ::Adder::state )
{
next_trigger(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
return ;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE1 )
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