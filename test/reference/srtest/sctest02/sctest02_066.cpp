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
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT1 = 2U,
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
};
void T();
bool proceed;
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
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT };
auto unsigned int state;
PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT:;
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE==state )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT1);
}
if(  ::Adder::T_STATE_PROCEED_NEXT1==state )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE1==state )
{
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
return ;
}
goto *(lmap[state]);
}

void Multiplier::T1()
{
static const unsigned int (lmap1[]) = { &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT };
auto unsigned int state1;
PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT:;
if( (sc_delta_count())==(0U) )
{
wait(SC_ZERO_TIME);
state1=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT2==state1 )
{
wait(SC_ZERO_TIME);
state1=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2==state1 )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state1=((!(! ::Multiplier::proceed1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3 :  ::Multiplier::T_STATE_PROCEED_NEXT3);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT3==state1 )
{
wait(SC_ZERO_TIME);
state1=((! ::Multiplier::proceed1) ?  ::Multiplier::T_STATE_PROCEED_NEXT3 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3);
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE3==state1 )
{
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state1=((!(! ::Multiplier::proceed1)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4 :  ::Multiplier::T_STATE_PROCEED_NEXT4);
}
if(  ::Multiplier::T_STATE_PROCEED_NEXT4==state1 )
{
wait(SC_ZERO_TIME);
state1=((! ::Multiplier::proceed1) ?  ::Multiplier::T_STATE_PROCEED_NEXT4 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4);
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE4==state1 )
{
 ::Multiplier::proceed1=(false);
cease(  ::gvar );
return ;
}
goto *(lmap1[state1]);
}

void TopLevel::T2()
{
static const unsigned int (lmap2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
