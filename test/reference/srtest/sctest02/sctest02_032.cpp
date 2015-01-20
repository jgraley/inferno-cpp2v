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
void T();
bool proceed;
bool instigate;
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
};
TopLevel top_level("top_level");

void Adder::T()
{
wait(SC_ZERO_TIME);
goto *((!(! ::Adder::proceed)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
goto *((! ::Adder::proceed) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
;
ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
goto *((!(! ::Adder::proceed)) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
NEXT_1:;
wait(SC_ZERO_TIME);
CONTINUE_1:;
goto *((! ::Adder::proceed) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
goto *((!(! ::Multiplier::instigate)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
goto *((! ::Multiplier::instigate) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
goto ELSE;
THEN:;
;
ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
NEXT_1:;
wait(SC_ZERO_TIME);
CONTINUE_1:;
goto *((! ::Multiplier::proceed) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN_2) : (&&PROCEED_4));
PROCEED_4:;
NEXT_2:;
wait(SC_ZERO_TIME);
CONTINUE_2:;
goto *((! ::Multiplier::proceed) ? (&&NEXT_2) : (&&PROCEED_5));
PROCEED_5:;
goto ELSE_2;
THEN_2:;
;
ELSE_2:;
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
