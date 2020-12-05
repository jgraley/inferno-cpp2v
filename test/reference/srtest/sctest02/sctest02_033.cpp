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
void T2();
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
SC_THREAD(T);
}
void T();
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
int gvar;
TopLevel top_level("top_level");

void Adder::T2()
{
wait(SC_ZERO_TIME);
goto *((!(! ::Adder::proceed)) ? (&&THEN3) : (&&PROCEED6));
PROCEED6:;
NEXT3:;
wait(SC_ZERO_TIME);
CONTINUE3:;
goto *((! ::Adder::proceed) ? (&&NEXT3) : (&&PROCEED7));
PROCEED7:;
goto ELSE3;
THEN3:;
ELSE3:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
goto *((!(! ::Adder::proceed)) ? (&&THEN4) : (&&PROCEED8));
PROCEED8:;
NEXT4:;
wait(SC_ZERO_TIME);
CONTINUE4:;
goto *((! ::Adder::proceed) ? (&&NEXT4) : (&&PROCEED9));
PROCEED9:;
goto ELSE4;
THEN4:;
ELSE4:;
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
return ;
}

void Multiplier::T1()
{
goto *((!(! ::Multiplier::instigate)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
goto *((! ::Multiplier::instigate) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed1)) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
goto *((! ::Multiplier::proceed1) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
goto ELSE1;
THEN1:;
ELSE1:;
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed1)) ? (&&THEN2) : (&&PROCEED4));
PROCEED4:;
NEXT2:;
wait(SC_ZERO_TIME);
CONTINUE2:;
goto *((! ::Multiplier::proceed1) ? (&&NEXT2) : (&&PROCEED5));
PROCEED5:;
goto ELSE2;
THEN2:;
ELSE2:;
 ::Multiplier::proceed1=(false);
cease(  ::gvar );
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}
