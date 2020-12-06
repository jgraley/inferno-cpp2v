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
sc_event proceed;
void T();
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
sc_event instigate;
sc_event proceed1;
void T1();
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
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD1 };
auto unsigned int state;
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD;
YIELD1:;
YIELD:;
if(  ::Adder::T_STATE_YIELD==state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
state= ::Adder::T_STATE_YIELD1;
goto *(lmap[state]);
}
if(  ::Adder::T_STATE_YIELD1==state )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
return ;
}
goto *(lmap[state]);
}

void Multiplier::T1()
{
static const unsigned int (lmap1[]) = { &&YIELD2, &&YIELD3, &&YIELD4 };
auto unsigned int state1;
wait(  ::Multiplier::instigate );
state1= ::Multiplier::T_STATE_YIELD2;
YIELD4:;
YIELD3:;
YIELD2:;
if(  ::Multiplier::T_STATE_YIELD2==state1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed1 );
state1= ::Multiplier::T_STATE_YIELD3;
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_YIELD3==state1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed1 );
state1= ::Multiplier::T_STATE_YIELD4;
goto *(lmap1[state1]);
}
if(  ::Multiplier::T_STATE_YIELD4==state1 )
{
cease(  ::gvar );
return ;
}
goto *(lmap1[state1]);
}

void TopLevel::T2()
{
static const unsigned int (lmap2[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
