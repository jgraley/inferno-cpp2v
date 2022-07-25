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
T_STATE_YIELD = 0,
T_STATE_YIELD_1 = 1,
};
sc_event proceed;
void T();
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
T_STATE_YIELD_2 = 0,
T_STATE_YIELD_3 = 1,
T_STATE_YIELD_4 = 2,
};
sc_event instigate;
sc_event proceed_1;
void T_1();
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
static const void *(lmap[]) = { &&YIELD, &&YIELD_1 };
auto void *state;
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD_1]);
goto *(state);
}
YIELD_1:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T_1()
{
static const void *(lmap_1[]) = { &&YIELD_2, &&YIELD_3, &&YIELD_4 };
auto void *state_1;
wait(  ::Multiplier::instigate );
{
state_1=(lmap_1[ ::Multiplier::T_STATE_YIELD_4]);
goto *(state_1);
}
YIELD_4:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed_1 );
{
state_1=(lmap_1[ ::Multiplier::T_STATE_YIELD_3]);
goto *(state_1);
}
YIELD_3:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed_1 );
{
state_1=(lmap_1[ ::Multiplier::T_STATE_YIELD_2]);
goto *(state_1);
}
YIELD_2:;
cease(  ::gvar );
return ;
}

void TopLevel::T_2()
{
static const void *(lmap_2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
