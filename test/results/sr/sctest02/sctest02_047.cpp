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
SC_THREAD(T_1);
}
void T_1();
bool instigate;
bool proceed_1;
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
void T_2();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
int gvar;

void Adder::T()
{
wait(SC_ZERO_TIME);
{
goto *((!(! ::Adder::proceed)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
{
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
goto *((! ::Adder::proceed) ? (&&NEXT) : (&&PROCEED_1));
PROCEED_1:;
}
goto ELSE;
THEN:;
;
ELSE:;
}
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
{
goto *((!(! ::Adder::proceed)) ? (&&THEN_1) : (&&PROCEED_2));
PROCEED_2:;
{
NEXT_1:;
wait(SC_ZERO_TIME);
CONTINUE_1:;
goto *((! ::Adder::proceed) ? (&&NEXT_1) : (&&PROCEED_3));
PROCEED_3:;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
return ;
}

void Multiplier::T_1()
{
{
goto *((!(! ::Multiplier::instigate)) ? (&&THEN_2) : (&&PROCEED_4));
PROCEED_4:;
{
NEXT_2:;
wait(SC_ZERO_TIME);
CONTINUE_2:;
goto *((! ::Multiplier::instigate) ? (&&NEXT_2) : (&&PROCEED_5));
PROCEED_5:;
}
goto ELSE_2;
THEN_2:;
;
ELSE_2:;
}
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
goto *((!(! ::Multiplier::proceed_1)) ? (&&THEN_3) : (&&PROCEED_6));
PROCEED_6:;
{
NEXT_3:;
wait(SC_ZERO_TIME);
CONTINUE_3:;
goto *((! ::Multiplier::proceed_1) ? (&&NEXT_3) : (&&PROCEED_7));
PROCEED_7:;
}
goto ELSE_3;
THEN_3:;
;
ELSE_3:;
}
 ::Multiplier::proceed_1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
goto *((!(! ::Multiplier::proceed_1)) ? (&&THEN_4) : (&&PROCEED_8));
PROCEED_8:;
{
NEXT_4:;
wait(SC_ZERO_TIME);
CONTINUE_4:;
goto *((! ::Multiplier::proceed_1) ? (&&NEXT_4) : (&&PROCEED_9));
PROCEED_9:;
}
goto ELSE_4;
THEN_4:;
;
ELSE_4:;
}
 ::Multiplier::proceed_1=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T_2()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
