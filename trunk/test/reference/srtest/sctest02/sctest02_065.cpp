#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
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
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 5U,
T_STATE_PROCEED_NEXT = 4U,
T_STATE_PROCEED_THEN_ELSE2 = 1U,
T_STATE_PROCEED_NEXT1 = 2U,
T_STATE_PROCEED_NEXT2 = 0U,
};
bool instigate;
};
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
void T();
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_NEXT1 = 2U,
};
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
TopLevel top_level("top_level");

void Multiplier::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT };
wait(SC_ZERO_TIME);
state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT:;
if( state== ::Multiplier::T_STATE_PROCEED_NEXT2 )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT1);
}
if( state== ::Multiplier::T_STATE_PROCEED_NEXT1 )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
if( state== ::Multiplier::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
}
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}
goto *(lmap[state]);
}

void Adder::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT, &&PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
PROCEED_THEN_ELSE_PROCEED_NEXT_PROCEED_THEN_ELSE_PROCEED_NEXT:;
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
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
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
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}
goto *(lmap[state]);
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
