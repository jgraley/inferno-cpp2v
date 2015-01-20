#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
bool instigate;
void T();
enum TStates
{
T_STATE_PROCEED_NEXT = 4U,
T_STATE_PROCEED_NEXT_1 = 2U,
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_NEXT_2 = 0U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_PROCEED_THEN_ELSE_2 = 5U,
};
private:
unsigned int state;
public:
bool proceed;
};
int gvar;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
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
void T();
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
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT_2);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT_2== ::Multiplier::state )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT_2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE== ::Multiplier::state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1 :  ::Multiplier::T_STATE_PROCEED_NEXT_1);
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT_1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1== ::Multiplier::state )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2);
continue;
}
if(  ::Multiplier::state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2 )
{
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void Adder::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE_1 :  ::Adder::T_STATE_PROCEED_NEXT_1);
continue;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT_1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE_1);
continue;
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
wait(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if(  ::Adder::state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
