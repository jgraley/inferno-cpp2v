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
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
void T();
sc_event proceed;
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
T_STATE_YIELD2 = 0U,
T_STATE_YIELD3 = 1U,
T_STATE_YIELD4 = 2U,
};
void T1();
sc_event instigate;
sc_event proceed1;
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
static const void *(lmap[]) = { &&YIELD, &&YIELD1 };
auto void *state;
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD1]);
goto *(state);
}
YIELD1:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T1()
{
static const void *(lmap1[]) = { &&YIELD2, &&YIELD3, &&YIELD4 };
auto void *state1;
wait(  ::Multiplier::instigate );
{
state1=(lmap1[ ::Multiplier::T_STATE_YIELD2]);
goto *(state1);
}
YIELD2:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed1 );
{
state1=(lmap1[ ::Multiplier::T_STATE_YIELD3]);
goto *(state1);
}
YIELD3:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed1 );
{
state1=(lmap1[ ::Multiplier::T_STATE_YIELD4]);
goto *(state1);
}
YIELD4:;
cease(  ::gvar );
return ;
}

void TopLevel::T2()
{
static const void *(lmap2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
