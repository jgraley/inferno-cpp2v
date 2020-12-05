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
T_STATE_YIELD3 = 0U,
T_STATE_YIELD4 = 1U,
};
void T2();
sc_event proceed1;
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
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 2U,
};
void T1();
sc_event instigate;
sc_event proceed;
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
static const unsigned int (lmap2[]) = { &&YIELD1, &&YIELD1 };
auto unsigned int state1;
wait(  ::Adder::proceed1 );
state1= ::Adder::T_STATE_YIELD3;
YIELD1:;
if(  ::Adder::T_STATE_YIELD3==state1 )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed1 );
state1= ::Adder::T_STATE_YIELD4;
goto *(lmap2[state1]);
}
if(  ::Adder::T_STATE_YIELD4==state1 )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap2[state1]);
}

void Multiplier::T1()
{
static const unsigned int (lmap1[]) = { &&YIELD, &&YIELD, &&YIELD };
auto unsigned int state;
wait(  ::Multiplier::instigate );
state= ::Multiplier::T_STATE_YIELD;
YIELD:;
if(  ::Multiplier::T_STATE_YIELD==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD1;
goto *(lmap1[state]);
}
if(  ::Multiplier::T_STATE_YIELD1==state )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed1).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
state= ::Multiplier::T_STATE_YIELD2;
goto *(lmap1[state]);
}
if(  ::Multiplier::T_STATE_YIELD2==state )
{
cease(  ::gvar );
return ;
}
goto *(lmap1[state]);
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
