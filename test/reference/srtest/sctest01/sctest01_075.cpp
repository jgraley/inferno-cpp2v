#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_METHOD(T);
}
enum TStates
{
T_STATE_YIELD = 0U,
T_STATE_YIELD1 = 1U,
};
void T();
sc_event proceed;
private:
unsigned int state;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_METHOD(T1);
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
private:
unsigned int state1;
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
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD;
return ;
}
if(  ::Adder::T_STATE_YIELD== ::Adder::state )
{
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
next_trigger(  ::Adder::proceed );
 ::Adder::state= ::Adder::T_STATE_YIELD1;
return ;
}
if(  ::Adder::T_STATE_YIELD1== ::Adder::state )
{
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1).notify(SC_ZERO_TIME);
return ;
}
next_trigger(SC_ZERO_TIME);
}

void Multiplier::T1()
{
if( (sc_delta_count())==(0U) )
{
next_trigger(  ::Multiplier::instigate );
 ::Multiplier::state1= ::Multiplier::T_STATE_YIELD2;
return ;
}
if(  ::Multiplier::T_STATE_YIELD2== ::Multiplier::state1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed1 );
 ::Multiplier::state1= ::Multiplier::T_STATE_YIELD3;
return ;
}
if(  ::Multiplier::T_STATE_YIELD3== ::Multiplier::state1 )
{
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed).notify(SC_ZERO_TIME);
next_trigger(  ::Multiplier::proceed1 );
 ::Multiplier::state1= ::Multiplier::T_STATE_YIELD4;
return ;
}
if(  ::Multiplier::T_STATE_YIELD4== ::Multiplier::state1 )
{
cease(  ::gvar );
return ;
}
next_trigger(SC_ZERO_TIME);
}

void TopLevel::T2()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate).notify(SC_ZERO_TIME);
return ;
}
