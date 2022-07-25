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
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
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
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
return ;
}

void Multiplier::T_1()
{
{
if( !(! ::Multiplier::instigate) )
goto THEN_2;
{
NEXT_2:;
wait(SC_ZERO_TIME);
CONTINUE_2:;
if( ! ::Multiplier::instigate )
goto NEXT_2;
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
if( !(! ::Multiplier::proceed_1) )
goto THEN_3;
{
NEXT_3:;
wait(SC_ZERO_TIME);
CONTINUE_3:;
if( ! ::Multiplier::proceed_1 )
goto NEXT_3;
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
if( !(! ::Multiplier::proceed_1) )
goto THEN_4;
{
NEXT_4:;
wait(SC_ZERO_TIME);
CONTINUE_4:;
if( ! ::Multiplier::proceed_1 )
goto NEXT_4;
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
