#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
bool proceed;
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT1 = 2U,
};
void T();
};
int gvar;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
bool proceed;
void T();
bool instigate;
enum TStates
{
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT2 = 4U,
T_STATE_PROCEED_THEN_ELSE = 5U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_THEN_ELSE2 = 1U,
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
void T();
enum TStates
{
};
 ::Multiplier mul_inst;
 ::Adder add_inst;
};
TopLevel top_level("top_level");

void Adder::T()
{
auto unsigned int state;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
continue;
}
if( state== ::Adder::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if( state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT1);
}
if(  ::Adder::T_STATE_PROCEED_NEXT1==state )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
continue;
}
if( state== ::Adder::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}
}
while( true );
}

void Multiplier::T()
{
auto unsigned int state;
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT1);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT1==state )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2==state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1==state )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT2);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT2==state )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE )
{
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}
}
while( true );
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}