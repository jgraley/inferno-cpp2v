#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
int gvar;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
void T();
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
sc_event instigate;
sc_event proceed;
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
T_STATE_YIELD2 = 2U,
};
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
 ::Multiplier mul_inst;
void T();
enum TStates
{
};
};
TopLevel top_level("top_level");

void Adder::T()
{
auto void *state;
static const void *(lmap[]) = { &&YIELD, &&YIELD1 };
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
wait(  ::Adder::proceed );
{
state=(lmap[ ::Adder::T_STATE_YIELD1]);
goto *(state);
}
YIELD1:;
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed).notify(SC_ZERO_TIME);
return ;
}

void Multiplier::T()
{
auto void *state;
static const void *(lmap[]) = { &&YIELD, &&YIELD1, &&YIELD2 };
wait(  ::Multiplier::instigate );
{
state=(lmap[ ::Multiplier::T_STATE_YIELD]);
goto *(state);
}
YIELD:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state=(lmap[ ::Multiplier::T_STATE_YIELD1]);
goto *(state);
}
YIELD1:;
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
wait(  ::Multiplier::proceed );
{
state=(lmap[ ::Multiplier::T_STATE_YIELD2]);
goto *(state);
}
YIELD2:;
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
static const void *(lmap[]) = {  };
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}