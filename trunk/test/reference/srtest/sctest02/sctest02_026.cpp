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
int gvar;
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T);
}
bool proceed;
void T();
bool instigate;
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
void T();
 ::Adder add_inst;
};
TopLevel top_level("top_level");

void Adder::T()
{
wait(SC_ZERO_TIME);
{
if( !(! ::Adder::proceed) )
goto THEN;
{
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
if( ! ::Adder::proceed )
goto NEXT;
}
goto ELSE;
THEN:;
;
ELSE:;
}
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
{
if( !(! ::Adder::proceed) )
goto THEN_1;
{
NEXT_1:;
wait(SC_ZERO_TIME);
CONTINUE_1:;
if( ! ::Adder::proceed )
goto NEXT_1;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void Multiplier::T()
{
{
if( !(! ::Multiplier::instigate) )
goto THEN;
{
NEXT:;
wait(SC_ZERO_TIME);
CONTINUE:;
if( ! ::Multiplier::instigate )
goto NEXT;
}
goto ELSE;
THEN:;
;
ELSE:;
}
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
if( !(! ::Multiplier::proceed) )
goto THEN_1;
{
NEXT_1:;
wait(SC_ZERO_TIME);
CONTINUE_1:;
if( ! ::Multiplier::proceed )
goto NEXT_1;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
if( !(! ::Multiplier::proceed) )
goto THEN_2;
{
NEXT_2:;
wait(SC_ZERO_TIME);
CONTINUE_2:;
if( ! ::Multiplier::proceed )
goto NEXT_2;
}
goto ELSE_2;
THEN_2:;
;
ELSE_2:;
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
