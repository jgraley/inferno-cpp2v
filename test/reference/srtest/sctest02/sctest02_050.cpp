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
static const void *(lmap[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1 };
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
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
{
state=((!(! ::Adder::proceed)) ? (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE1]) : (lmap[ ::Adder::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (lmap[ ::Adder::T_STATE_PROCEED_NEXT1]) : (lmap[ ::Adder::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
return ;
}

void Multiplier::T1()
{
static const void *(lmap1[]) = { &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE2, &&PROCEED_NEXT3, &&PROCEED_THEN_ELSE3, &&PROCEED_NEXT4, &&PROCEED_THEN_ELSE4 };
auto void *state1;
wait(SC_ZERO_TIME);
{
state1=((!(! ::Multiplier::instigate)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT2]));
goto *(state1);
}
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
{
state1=((! ::Multiplier::instigate) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT2]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]));
goto *(state1);
}
PROCEED_THEN_ELSE2:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state1=((!(! ::Multiplier::proceed1)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE3]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT3]));
goto *(state1);
}
PROCEED_NEXT3:;
wait(SC_ZERO_TIME);
{
state1=((! ::Multiplier::proceed1) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT3]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE3]));
goto *(state1);
}
PROCEED_THEN_ELSE3:;
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state1=((!(! ::Multiplier::proceed1)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE4]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT4]));
goto *(state1);
}
PROCEED_NEXT4:;
wait(SC_ZERO_TIME);
{
state1=((! ::Multiplier::proceed1) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT4]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE4]));
goto *(state1);
}
PROCEED_THEN_ELSE4:;
 ::Multiplier::proceed1=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T2()
{
static const void *(lmap2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
