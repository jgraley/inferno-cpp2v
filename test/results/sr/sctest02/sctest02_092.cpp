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
enum TStates
{
T_STATE_PROCEED_NEXT = 0,
T_STATE_PROCEED_NEXT_1 = 2,
T_STATE_PROCEED_THEN_ELSE = 1,
T_STATE_PROCEED_THEN_ELSE_1 = 3,
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
SC_THREAD(T_1);
}
enum TStates_1
{
T_STATE_PROCEED_NEXT_2 = 1,
T_STATE_PROCEED_NEXT_3 = 2,
T_STATE_PROCEED_NEXT_4 = 4,
T_STATE_PROCEED_THEN_ELSE_2 = 0,
T_STATE_PROCEED_THEN_ELSE_3 = 3,
T_STATE_PROCEED_THEN_ELSE_4 = 5,
};
void T_1();
bool instigate;
bool proceed_1;
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
do
{
if( (sc_delta_count())==(0) )
{
wait(SC_ZERO_TIME);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE_1 :  ::Adder::T_STATE_PROCEED_NEXT);
continue;
}
if(  ::Adder::T_STATE_PROCEED_NEXT== ::Adder::state )
{
wait(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE_1);
continue;
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE_1== ::Adder::state )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
 ::Adder::state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT_1);
}
if(  ::Adder::T_STATE_PROCEED_NEXT_1== ::Adder::state )
{
wait(SC_ZERO_TIME);
 ::Adder::state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT_1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE== ::Adder::state )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void Multiplier::T_1()
{
do
{
if( (sc_delta_count())==(0) )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state_1=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2 :  ::Multiplier::T_STATE_PROCEED_NEXT_2);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT_2== ::Multiplier::state_1 )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state_1=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT_2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2== ::Multiplier::state_1 )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state_1=((!(! ::Multiplier::proceed_1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_4 :  ::Multiplier::T_STATE_PROCEED_NEXT_4);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT_4== ::Multiplier::state_1 )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state_1=((! ::Multiplier::proceed_1) ?  ::Multiplier::T_STATE_PROCEED_NEXT_4 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_4);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_4== ::Multiplier::state_1 )
{
 ::Multiplier::proceed_1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
 ::Multiplier::state_1=((!(! ::Multiplier::proceed_1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_3 :  ::Multiplier::T_STATE_PROCEED_NEXT_3);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT_3== ::Multiplier::state_1 )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state_1=((! ::Multiplier::proceed_1) ?  ::Multiplier::T_STATE_PROCEED_NEXT_3 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_3);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_3== ::Multiplier::state_1 )
{
 ::Multiplier::proceed_1=(false);
cease(  ::gvar );
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void TopLevel::T_2()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}