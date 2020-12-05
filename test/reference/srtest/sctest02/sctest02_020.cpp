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
if( ! ::Adder::proceed )
{
NEXT3:;
wait(SC_ZERO_TIME);
CONTINUE3:;
if( ! ::Adder::proceed )
goto NEXT3;
}
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
if( ! ::Adder::proceed )
{
NEXT4:;
wait(SC_ZERO_TIME);
CONTINUE4:;
if( ! ::Adder::proceed )
goto NEXT4;
}
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed1)=(true);
return ;
}

void Multiplier::T1()
{
if( ! ::Multiplier::instigate )
{
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
if( ! ::Multiplier::instigate )
goto NEXT;
}
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
if( ! ::Multiplier::proceed1 )
{
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
if( ! ::Multiplier::proceed1 )
goto NEXT1;
}
 ::Multiplier::proceed1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
if( ! ::Multiplier::proceed1 )
{
NEXT2:;
wait(SC_ZERO_TIME);
CONTINUE2:;
if( ! ::Multiplier::proceed1 )
goto NEXT2;
}
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
