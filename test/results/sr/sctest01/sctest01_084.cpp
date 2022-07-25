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
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD };
auto unsigned int state;
YIELD:;
if( (sc_delta_count())==(0) )
{
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_YIELD==state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD_1;
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_YIELD_1==state )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap[state]);
}

void Multiplier::T_1()
{
static const unsigned int (lmap_1[]) = { &&YIELD_1, &&YIELD_1, &&YIELD_1 };
auto unsigned int state_1;
YIELD_1:;
if( (sc_delta_count())==(0) )
{
wait(  ::Multiplier::instigate );
state_1= ::Multiplier::T_STATE_YIELD_4;
goto *(lmap_1[state_1]);
}
if(  ::Multiplier::T_STATE_YIELD_4==state_1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed_1 );
state_1= ::Multiplier::T_STATE_YIELD_3;
goto *(lmap_1[state_1]);
}
if(  ::Multiplier::T_STATE_YIELD_3==state_1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed_1 );
state_1= ::Multiplier::T_STATE_YIELD_2;
goto *(lmap_1[state_1]);
}
if(  ::Multiplier::T_STATE_YIELD_2==state_1 )
{
cease(  ::gvar );
return ;
}
goto *(lmap_1[state_1]);
}

void TopLevel::T_2()
{
static const unsigned int (lmap_2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
