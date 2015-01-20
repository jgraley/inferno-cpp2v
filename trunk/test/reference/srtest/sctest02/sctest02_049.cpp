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
bool proceed;
bool instigate;
void T();
};
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
SC_THREAD(T);
}
void T();
bool proceed;
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
mul_inst("mul_inst"),
add_inst("add_inst")
{
SC_THREAD(T);
}
 ::Multiplier mul_inst;
 ::Adder add_inst;
void T();
};
TopLevel top_level("top_level");
int gvar;

void Multiplier::T()
{
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Multiplier::instigate)) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::instigate) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT_1));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_1));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
state=((!(! ::Multiplier::proceed)) ? (&&PROCEED_THEN_ELSE_2) : (&&PROCEED_NEXT_2));
goto *(state);
}
PROCEED_NEXT_2:;
wait(SC_ZERO_TIME);
{
state=((! ::Multiplier::proceed) ? (&&PROCEED_NEXT_2) : (&&PROCEED_THEN_ELSE_2));
goto *(state);
}
PROCEED_THEN_ELSE_2:;
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

void Adder::T()
{
auto void *state;
wait(SC_ZERO_TIME);
{
state=((!(! ::Adder::proceed)) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
goto *(state);
}
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto *(state);
}
PROCEED_THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
{
state=((!(! ::Adder::proceed)) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT_1));
goto *(state);
}
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
{
state=((! ::Adder::proceed) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_1));
goto *(state);
}
PROCEED_THEN_ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
