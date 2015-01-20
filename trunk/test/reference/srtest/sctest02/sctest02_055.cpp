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
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_PROCEED_NEXT_1 = 0U,
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
bool instigate;
void T();
bool proceed;
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE_1 = 5U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_NEXT_1 = 2U,
T_STATE_PROCEED_THEN_ELSE_2 = 3U,
T_STATE_PROCEED_NEXT_2 = 4U,
};
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
 ::Adder add_inst;
void T();
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");

void Adder::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_1 };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT_1);
PROCEED_NEXT:;
if( state== ::Adder::T_STATE_PROCEED_NEXT_1 )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT_1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
if( state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE_1 :  ::Adder::T_STATE_PROCEED_NEXT);
}
goto *(lmap[state]);
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
PROCEED_THEN_ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT_2, &&PROCEED_THEN_ELSE_2 };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if(  ::Multiplier::T_STATE_PROCEED_NEXT==state )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
if(  ::Multiplier::T_STATE_PROCEED_THEN_ELSE==state )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2 :  ::Multiplier::T_STATE_PROCEED_NEXT_1);
}
goto *(lmap[state]);
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT_1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2);
goto *(lmap[state]);
PROCEED_THEN_ELSE_1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1 :  ::Multiplier::T_STATE_PROCEED_NEXT_2);
goto *(lmap[state]);
PROCEED_NEXT_2:;
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT_2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1);
goto *(lmap[state]);
PROCEED_THEN_ELSE_2:;
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
