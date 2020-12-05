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
static const void *(lmap2[]) = { &&PROCEED_NEXT3, &&PROCEED_THEN_ELSE3, &&PROCEED_NEXT4, &&PROCEED_THEN_ELSE4 };
auto void *state1;
wait(SC_ZERO_TIME);
{
state1=((!(! ::Adder::proceed1)) ? (lmap2[ ::Adder::T_STATE_PROCEED_THEN_ELSE3]) : (lmap2[ ::Adder::T_STATE_PROCEED_NEXT4]));
goto *(state1);
}
PROCEED_NEXT3:;
wait(SC_ZERO_TIME);
{
state1=((! ::Adder::proceed1) ? (lmap2[ ::Adder::T_STATE_PROCEED_NEXT4]) : (lmap2[ ::Adder::T_STATE_PROCEED_THEN_ELSE3]));
goto *(state1);
}
PROCEED_THEN_ELSE3:;
 ::Adder::proceed1=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
{
state1=((!(! ::Adder::proceed1)) ? (lmap2[ ::Adder::T_STATE_PROCEED_THEN_ELSE4]) : (lmap2[ ::Adder::T_STATE_PROCEED_NEXT3]));
goto *(state1);
}
PROCEED_NEXT4:;
wait(SC_ZERO_TIME);
{
state1=((! ::Adder::proceed1) ? (lmap2[ ::Adder::T_STATE_PROCEED_NEXT3]) : (lmap2[ ::Adder::T_STATE_PROCEED_THEN_ELSE4]));
goto *(state1);
}
PROCEED_THEN_ELSE4:;
 ::Adder::proceed1=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T1()
{
static const void *(lmap1[]) = { &&PROCEED_NEXT, &&PROCEED_THEN_ELSE, &&PROCEED_NEXT1, &&PROCEED_THEN_ELSE1, &&PROCEED_NEXT2, &&PROCEED_THEN_ELSE2 };
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Multiplier::instigate)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE1]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT1]));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::instigate) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT1]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE1]));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT2]));
goto *(state);
}
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT2]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE]));
goto *(state);
}
PROCEED_THEN_ELSE1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT]));
goto *(state);
}
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (lmap1[ ::Multiplier::T_STATE_PROCEED_NEXT]) : (lmap1[ ::Multiplier::T_STATE_PROCEED_THEN_ELSE2]));
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
