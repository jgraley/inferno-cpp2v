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
SC_THREAD(T);
}
enum TStates
{
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT = 2U,
T_STATE_PROCEED_NEXT2 = 4U,
T_STATE_PROCEED_THEN_ELSE = 1U,
T_STATE_PROCEED_THEN_ELSE1 = 3U,
T_STATE_PROCEED_THEN_ELSE2 = 5U,
};
void T();
bool instigate;
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
enum TStates
{
};
void T();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1 };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE :  ::Adder::T_STATE_PROCEED_NEXT);
PROCEED_NEXT:;
if( state== ::Adder::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT :  ::Adder::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
if( state== ::Adder::T_STATE_PROCEED_THEN_ELSE )
{
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
state=((!(! ::Adder::proceed)) ?  ::Adder::T_STATE_PROCEED_THEN_ELSE1 :  ::Adder::T_STATE_PROCEED_NEXT1);
}
PROCEED_NEXT1:;
if( state== ::Adder::T_STATE_PROCEED_NEXT1 )
{
wait(SC_ZERO_TIME);
state=((! ::Adder::proceed) ?  ::Adder::T_STATE_PROCEED_NEXT1 :  ::Adder::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
}
goto *(lmap[state]);
PROCEED_THEN_ELSE1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
static const unsigned int (lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE2 };
auto unsigned int state;
wait(SC_ZERO_TIME);
state=((!(! ::Multiplier::instigate)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE :  ::Multiplier::T_STATE_PROCEED_NEXT1);
PROCEED_NEXT:;
if( state== ::Multiplier::T_STATE_PROCEED_NEXT1 )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::instigate) ?  ::Multiplier::T_STATE_PROCEED_NEXT1 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE:;
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE )
{
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 :  ::Multiplier::T_STATE_PROCEED_NEXT);
}
PROCEED_NEXT1:;
if( state== ::Multiplier::T_STATE_PROCEED_NEXT )
{
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE1);
goto *(lmap[state]);
}
PROCEED_THEN_ELSE1:;
if( state== ::Multiplier::T_STATE_PROCEED_THEN_ELSE1 )
{
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
state=((!(! ::Multiplier::proceed)) ?  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2 :  ::Multiplier::T_STATE_PROCEED_NEXT2);
}
goto *(lmap[state]);
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
state=((! ::Multiplier::proceed) ?  ::Multiplier::T_STATE_PROCEED_NEXT2 :  ::Multiplier::T_STATE_PROCEED_THEN_ELSE2);
goto *(lmap[state]);
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
