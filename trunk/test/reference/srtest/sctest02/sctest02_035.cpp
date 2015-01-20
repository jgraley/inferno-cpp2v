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
void T();
bool proceed;
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
bool instigate;
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

void Adder::T()
{
wait(SC_ZERO_TIME);
goto *((!(! ::Adder::proceed)) ? (&&THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
goto *((! ::Adder::proceed) ? (&&PROCEED_NEXT) : (&&PROCEED));
PROCEED:;
goto THEN_ELSE;
THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
goto *((!(! ::Adder::proceed)) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
goto *((! ::Adder::proceed) ? (&&PROCEED_NEXT_1) : (&&PROCEED_1));
PROCEED_1:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
goto *((!(! ::Multiplier::instigate)) ? (&&THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::instigate) ? (&&PROCEED_NEXT) : (&&PROCEED));
PROCEED:;
goto THEN_ELSE;
THEN_ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN_ELSE_1) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::proceed) ? (&&PROCEED_NEXT_1) : (&&PROCEED_1));
PROCEED_1:;
goto THEN_ELSE_1;
THEN_ELSE_1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN_ELSE_2) : (&&PROCEED_NEXT_2));
PROCEED_NEXT_2:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::proceed) ? (&&PROCEED_NEXT_2) : (&&PROCEED_2));
PROCEED_2:;
goto THEN_ELSE_2;
THEN_ELSE_2:;
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
