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
static const unsigned int (lmap2[]) = { &&PROCEED_NEXT3, &&PROCEED_THEN_ELSE3, &&PROCEED_NEXT4, &&PROCEED_THEN_ELSE4 };
auto unsigned int state1;
wait(SC_ZERO_TIME);
state1=((!(! ::Adder::proceed1)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE3 :  ::Adder::T_STATE_PROCEED_NEXT4);
PROCEED_NEXT3:;
if(  ::Adder::T_STATE_PROCEED_NEXT4==state1 )
{
wait(SC_ZERO_TIME);
state1=((! ::Adder::proceed1) ?  ::Adder::T_STATE_PROCEED_NEXT4 :  ::Adder::T_STATE_PROCEED_THEN_ELSE3);
goto *(lmap2[state1]);
}
PROCEED_THEN_ELSE3:;
if(  ::Adder::T_STATE_PROCEED_THEN_ELSE3==state1 )
{
 ::Adder::proceed1=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
state1=((!(! ::Adder::proceed1)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE4 :  ::Adder::T_STATE_PROCEED_NEXT3);
}
goto *(lmap2[state1]);
PROCEED_NEXT4:;
wait(SC_ZERO_TIME);
state1=((! ::Adder::proceed1) ?  ::Adder::T_STATE_PROCEED_NEXT3 :  ::Adder::T_STATE_PROCEED_THEN_ELSE4);
goto *(lmap2[state1]);
PROCEED_THEN_ELSE4:;
 ::Adder::proceed1=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T1()
{
static const unsigned int (lmap1[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE2 };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT1);
PROCEED_NEXT:;
if(  ::Multiplier::T_STATE_PROCEED_NEXT1==state )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap1[state]);
}
PROCEED_THEN_ELSE:;
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1==state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT2);
}
goto *(lmap1[state]);
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap1[state]);
PROCEED_THEN_ELSE1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT);
goto *(lmap1[state]);
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
goto *(lmap1[state]);
PROCEED_THEN_ELSE2:;
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
