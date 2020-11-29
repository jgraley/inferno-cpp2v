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
bool proceed;
void T();
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
bool instigate;
bool proceed;
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
};
int gvar;
TopLevel top_level("top_level");

void Adder::T()
{
wait(SC_ZERO_TIME);
goto *((!(! ::Adder::proceed)) ? (&&THEN) : (&&PROCEED));
PROCEED:;
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
goto *((! ::Adder::proceed) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
ELSE:;
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
goto *((!(! ::Adder::proceed)) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
goto *((! ::Adder::proceed) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
goto ELSE1;
THEN1:;
ELSE1:;
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
goto *((! ::Multiplier::instigate) ? (&&NEXT) : (&&PROCEED1));
PROCEED1:;
goto ELSE;
THEN:;
ELSE:;
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN1) : (&&PROCEED2));
PROCEED2:;
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
goto *((! ::Multiplier::proceed) ? (&&NEXT1) : (&&PROCEED3));
PROCEED3:;
goto ELSE1;
THEN1:;
ELSE1:;
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
goto *((!(! ::Multiplier::proceed)) ? (&&THEN2) : (&&PROCEED4));
PROCEED4:;
NEXT2:;
wait(SC_ZERO_TIME);
CONTINUE2:;
goto *((! ::Multiplier::proceed) ? (&&NEXT2) : (&&PROCEED5));
PROCEED5:;
goto ELSE2;
THEN2:;
ELSE2:;
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
