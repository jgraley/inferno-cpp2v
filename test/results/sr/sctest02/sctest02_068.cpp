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
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT_1, &&PROCEED_THEN_ELSE_1 };
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE_1]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE_1]));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT_1]));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT_1]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
return ;
}

void Multiplier::T_1()
{
static const void *(lmap_1[]) = { &&PROCEED_THEN_ELSE_2, &&PROCEED_NEXT_2, &&PROCEED_NEXT_3, &&PROCEED_THEN_ELSE_3, &&PROCEED_NEXT_4, &&PROCEED_THEN_ELSE_4 };
auto void *state_1;
wait(SC_ZERO_TIME);
{
state_1=((!(! ::Multiplier::instigate)) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_2]));
goto *(state_1);
}
PROCEED_NEXT_2:;
wait(SC_ZERO_TIME);
{
state_1=((! ::Multiplier::instigate) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_2]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_2]));
goto *(state_1);
}
PROCEED_THEN_ELSE_2:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state_1=((!(! ::Multiplier::proceed_1)) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_4]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_4]));
goto *(state_1);
}
PROCEED_NEXT_4:;
wait(SC_ZERO_TIME);
{
state_1=((! ::Multiplier::proceed_1) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_4]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_4]));
goto *(state_1);
}
PROCEED_THEN_ELSE_4:;
 ::Multiplier::proceed_1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state_1=((!(! ::Multiplier::proceed_1)) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_3]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_3]));
goto *(state_1);
}
PROCEED_NEXT_3:;
wait(SC_ZERO_TIME);
{
state_1=((! ::Multiplier::proceed_1) ? (lmap_1[ ::Multiplier::T_STATE_PROCEED_NEXT_3]) : (lmap_1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE_3]));
goto *(state_1);
}
PROCEED_THEN_ELSE_3:;
 ::Multiplier::proceed_1=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T_2()
{
static const void *(lmap_2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
