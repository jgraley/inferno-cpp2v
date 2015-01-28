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
while( ! ::Adder::proceed )
wait(SC_ZERO_TIME);
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
while( ! ::Adder::proceed )
wait(SC_ZERO_TIME);
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
while( ! ::Multiplier::instigate )
wait(SC_ZERO_TIME);
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
while( ! ::Multiplier::proceed )
wait(SC_ZERO_TIME);
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
while( ! ::Multiplier::proceed )
wait(SC_ZERO_TIME);
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
