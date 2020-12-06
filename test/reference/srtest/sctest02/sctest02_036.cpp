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
SC_THREAD(T1);
}
void T1();
bool instigate;
bool proceed1;
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
void T2();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
wait(SC_ZERO_TIME);
goto *((!(! ::Adder::proceed)) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
wait(SC_ZERO_TIME);
goto *((! ::Adder::proceed) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
PROCEED_THEN_ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
goto *((!(! ::Adder::proceed)) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
goto *((! ::Adder::proceed) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
goto PROCEED_THEN_ELSE1;
PROCEED_THEN_ELSE1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
return ;
}

void Multiplier::T1()
{
goto *((!(! ::Multiplier::instigate)) ? (&&PROCEED_THEN_ELSE2) : (&&PROCEED_NEXT2));
PROCEED_NEXT2:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::instigate) ? (&&PROCEED_NEXT2) : (&&PROCEED_THEN_ELSE2));
goto PROCEED_THEN_ELSE2;
PROCEED_THEN_ELSE2:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed1)) ? (&&PROCEED_THEN_ELSE3) : (&&PROCEED_NEXT3));
PROCEED_NEXT3:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::proceed1) ? (&&PROCEED_NEXT3) : (&&PROCEED_THEN_ELSE3));
goto PROCEED_THEN_ELSE3;
PROCEED_THEN_ELSE3:;
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed1)) ? (&&PROCEED_THEN_ELSE4) : (&&PROCEED_NEXT4));
PROCEED_NEXT4:;
wait(SC_ZERO_TIME);
goto *((! ::Multiplier::proceed1) ? (&&PROCEED_NEXT4) : (&&PROCEED_THEN_ELSE4));
goto PROCEED_THEN_ELSE4;
PROCEED_THEN_ELSE4:;
 ::Multiplier::proceed1=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T2()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
