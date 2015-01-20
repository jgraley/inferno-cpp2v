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
int gvar;

void Adder::T()
{
wait(SC_ZERO_TIME);
if( ! ::Adder::proceed )
do
wait(SC_ZERO_TIME);
while( ! ::Adder::proceed );
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
if( ! ::Adder::proceed )
do
wait(SC_ZERO_TIME);
while( ! ::Adder::proceed );
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
if( ! ::Multiplier::instigate )
do
wait(SC_ZERO_TIME);
while( ! ::Multiplier::instigate );
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
if( ! ::Multiplier::proceed )
do
wait(SC_ZERO_TIME);
while( ! ::Multiplier::proceed );
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
if( ! ::Multiplier::proceed )
do
wait(SC_ZERO_TIME);
while( ! ::Multiplier::proceed );
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
