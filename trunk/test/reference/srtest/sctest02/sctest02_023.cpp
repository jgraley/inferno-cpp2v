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
bool instigate;
bool proceed;
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
TopLevel top_level("top_level");

void Adder::T()
{
wait(SC_ZERO_TIME);
if( ! ::Adder::proceed )
{
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
if( ! ::Adder::proceed )
goto NEXT;
}
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
if( ! ::Adder::proceed )
{
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
if( ! ::Adder::proceed )
goto NEXT1;
}
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
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
if( ! ::Multiplier::proceed )
{
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
if( ! ::Multiplier::proceed )
goto NEXT1;
}
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
if( ! ::Multiplier::proceed )
{
NEXT2:;
wait(SC_ZERO_TIME);
CONTINUE2:;
if( ! ::Multiplier::proceed )
goto NEXT2;
}
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
