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
bool instigate;
enum TStates
{
T_STATE_PROCEED_THEN_ELSE = 3U,
T_STATE_PROCEED_NEXT = 0U,
T_STATE_PROCEED_THEN_ELSE_1 = 1U,
T_STATE_PROCEED_NEXT_1 = 2U,
T_STATE_PROCEED_NEXT_2 = 4U,
T_STATE_PROCEED_THEN_ELSE_2 = 5U,
};
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
T_STATE_PROCEED_THEN_ELSE_1 = 3U,
T_STATE_PROCEED_NEXT_1 = 2U,
};
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
 ::Multiplier mul_inst;
 ::Adder add_inst;
enum TStates
{
};
void T();
};
TopLevel top_level("top_level");

void Multiplier::T()
{
auto void *state;
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_1, &&PROCEED_NEXT_2, &&PROCEED_THEN_ELSE_2 };
wait(SC_ZERO_TIME);
{
state=((!(! ::Multiplier::instigate)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::instigate) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2]) : (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT_2]));
goto *(state);
}
PROCEED_NEXT_2:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap[ ::Multiplier::T_STATE_PROCEED_NEXT_2]) : (lmap[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2]));
goto *(state);
}
PROCEED_THEN_ELSE_2:;
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

void Adder::T()
{
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_1 };
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void TopLevel::T()
{
static const void *(lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
