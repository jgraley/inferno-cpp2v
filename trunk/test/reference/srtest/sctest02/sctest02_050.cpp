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
void T();
bool proceed;
enum TStates
{
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 1U,
T_STATE_PROCEED_NEXT1 = 2U,
};
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
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_THEN_ELSE1 = 1U,
T_STATE_PROCEED_THEN_ELSE2 = 5U,
T_STATE_PROCEED_NEXT = 4U,
T_STATE_PROCEED_NEXT1 = 0U,
T_STATE_PROCEED_NEXT2 = 2U,
};
bool proceed;
void T();
bool instigate;
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
 ::Adder add_inst;
void T();
enum TStates
{
};
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
auto void *state;
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1 };
wait(SC_ZERO_TIME);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE1]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT1]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE2 };
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Multiplier::instigate)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE1]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::instigate) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT1]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT2]));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT2]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]));
goto *(state);
}
PROCEED_THEN_ELSE2:;
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
static const void *(lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
