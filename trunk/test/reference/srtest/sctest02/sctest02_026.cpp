#include "isystemc.h"

class Multiplier;
class Adder;
class TopLevel;
int gvar;
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
void T();
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");

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
goto THEN1;
{
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
if( ! ::Multiplier::proceed )
goto NEXT1;
}
goto ELSE1;
THEN1:;
;
ELSE1:;
}
 ::Multiplier::proceed=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
{
if( !(! ::Multiplier::proceed) )
goto THEN2;
{
NEXT2:;
wait(SC_ZERO_TIME);
CONTINUE2:;
if( ! ::Multiplier::proceed )
goto NEXT2;
}
goto ELSE2;
THEN2:;
;
ELSE2:;
}
 ::Multiplier::proceed=(false);
cease(  ::gvar );
return ;
}

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
goto THEN1;
{
NEXT1:;
wait(SC_ZERO_TIME);
CONTINUE1:;
if( ! ::Adder::proceed )
goto NEXT1;
}
goto ELSE1;
THEN1:;
;
ELSE1:;
}
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed)=(true);
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
return ;
}