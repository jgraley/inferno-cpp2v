#include "isystemc.h"

class Adder;
class Multiplier;
class TopLevel;
class Adder : public sc_module
{
public:
SC_CTOR( Adder )
{
}
Adder(auto char (*name));
bool proceed;
void (T)();
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
}
Multiplier(auto char (*name_1));
bool instigate;
bool proceed_1;
void (T_1)();
};
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
add_inst("add_inst"),
mul_inst("mul_inst")
{
}
TopLevel(auto char (*name_2));
 ::Adder add_inst;
 ::Multiplier mul_inst;
void (T_2)();
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
int gvar;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);
void (SC_METHOD)(void func_1);
void (SC_THREAD)(void func);
void (SC_CTHREAD)(void clock, void func_2);

Adder::Adder(char (*name))
{
 ::SC_THREAD( ::Adder::T);
}

void (Adder::T)()
{
wait(SC_ZERO_TIME);
while( ! ::Adder::proceed )
wait(SC_ZERO_TIME);
 ::Adder::proceed=(false);
 ::gvar+=(2);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
while( ! ::Adder::proceed )
wait(SC_ZERO_TIME);
 ::Adder::proceed=(false);
 ::gvar+=(3);
(( ::top_level. ::TopLevel::mul_inst). ::Multiplier::proceed_1)=(true);
}

Multiplier::Multiplier(char (*name_1))
{
 ::SC_THREAD( ::Multiplier::T_1);
}

void (Multiplier::T_1)()
{
while( ! ::Multiplier::instigate )
wait(SC_ZERO_TIME);
 ::Multiplier::instigate=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
while( ! ::Multiplier::proceed_1 )
wait(SC_ZERO_TIME);
 ::Multiplier::proceed_1=(false);
 ::gvar*=(5);
(( ::top_level. ::TopLevel::add_inst). ::Adder::proceed)=(true);
while( ! ::Multiplier::proceed_1 )
wait(SC_ZERO_TIME);
 ::Multiplier::proceed_1=(false);
cease(  ::gvar );
}

TopLevel::TopLevel(char (*name_2)) : add_inst("add_inst"), mul_inst("mul_inst")
{
 ::SC_THREAD( ::TopLevel::T_2);
}

void (TopLevel::T_2)()
{
 ::gvar=(1);
( ::TopLevel::mul_inst. ::Multiplier::instigate)=(true);
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);

void (SC_METHOD)(void func_1);

void (SC_THREAD)(void func);

void (SC_CTHREAD)(void clock, void func_2);
