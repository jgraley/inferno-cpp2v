#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T2);
}
enum TStates2
{
T_STATE_PROCEED_NEXT4 = 0U,
T_STATE_PROCEED_NEXT3 = 2U,
T_STATE_PROCEED_THEN_ELSE3 = 1U,
T_STATE_PROCEED_THEN_ELSE4 = 3U,
};
void T2();
bool proceed1;
private:
unsigned int state1;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T1);
}
enum TStates1
{
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT2 = 2U,
T_STATE_PROCEED_NEXT = 4U,
T_STATE_PROCEED_THEN_ELSE1 = 1U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE2 = 5U,
};
void T1();
bool instigate;
bool proceed;
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
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
 ::Adder::state1=((!(! ::Adder::proceed1)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE3 :  ::Adder::T_STATE_PROCEED_NEXT4);
continue;
}
if(  ::Adder::T_STATE_PROCEED_NEXT4== ::Adder::state1 )
{
wait(SC_ZERO_TIME);
 ::Adder::state1=((! ::Adder::proceed1) ?  ::Adder::T_STATE_PROCEED_NEXT4 :  ::Adder::T_STATE_PROCEED_THEN_ELSE3);
continue;
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE3== ::Adder::state1 )
{
 ::Adder::proceed1=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
 ::Adder::state1=((!(! ::Adder::proceed1)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE4 :  ::Adder::T_STATE_PROCEED_NEXT3);
}
if(  ::Adder::T_STATE_PROCEED_NEXT3== ::Adder::state1 )
{
wait(SC_ZERO_TIME);
 ::Adder::state1=((! ::Adder::proceed1) ?  ::Adder::T_STATE_PROCEED_NEXT3 :  ::Adder::T_STATE_PROCEED_THEN_ELSE4);
continue;
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE4== ::Adder::state1 )
{
 ::Adder::proceed1=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}
wait(SC_ZERO_TIME);
}
while( true );
}

void Multiplier::T1()
{
do
{
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT1);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT1== ::Multiplier::state )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1== ::Multiplier::state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT2);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT2== ::Multiplier::state )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE== ::Multiplier::state )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
 ::Multiplier::state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT== ::Multiplier::state )
{
wait(SC_ZERO_TIME);
 ::Multiplier::state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
continue;
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2== ::Multiplier::state )
{
 ::Multiplier::proceed=(false);
cease(  ::gvar );
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
