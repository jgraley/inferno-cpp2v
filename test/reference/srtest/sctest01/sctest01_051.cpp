#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD1 = 0U,
T_STATE_YIELD2 = 2U,
};
void T();
sc_event instigate;
sc_event proceed;
};
int gvar;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_YIELD = 1U,
T_STATE_YIELD1 = 0U,
};
sc_event proceed;
void T();
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
enum TStates
{
};
void T();
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");

void Multiplier::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD1, &&YIELD2 };
wait(  ::Multiplier::instigate );
{
state= ::Multiplier::T_STATE_YIELD1;
goto *(lmap[state]);
}
YIELD:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state= ::Multiplier::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD1:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state= ::Multiplier::T_STATE_YIELD2;
goto *(lmap[state]);
}
YIELD2:;
cease(  ::gvar );
return ;
}

void Adder::T()
{
auto unsigned int state;
static const unsigned int (lmap[]) = { &&YIELD, &&YIELD1 };
wait(  ::Adder::proceed );
{
state= ::Adder::T_STATE_YIELD1;
goto *(lmap[state]);
}
YIELD:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
{
state= ::Adder::T_STATE_YIELD;
goto *(lmap[state]);
}
YIELD1:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}

void TopLevel::T()
{
static const unsigned int (lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
