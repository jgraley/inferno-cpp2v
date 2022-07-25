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
sc_event proceed;
void T();
Adder(auto char (*name));
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
SC_THREAD(T_1);
}
sc_event instigate;
sc_event proceed_1;
void T_1();
Multiplier(auto char (*name_1));
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
TopLevel(auto char (*name_2));
 ::Adder add_inst;
 ::Multiplier mul_inst;
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
int gvar;
void (cease)(void p1_1);
void (exit)(void p1_2);
void (next_trigger)(void p1_3);
void (wait)(void p1_4);
void (SC_METHOD)(void func);

void Adder::T()
{
wait(  ::Adder::proceed );
 ::gvar+=(2);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::Adder::proceed );
 ::gvar+=(3);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
}

Adder::Adder(char (*name))
{
}

void Multiplier::T_1()
{
wait(  ::Multiplier::instigate );
 ::gvar*=(5);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::Multiplier::proceed_1 );
 ::gvar*=(5);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
wait(  ::Multiplier::proceed_1 );
cease(  ::gvar );
}

Multiplier::Multiplier(char (*name_1))
{
}

void TopLevel::T_2()
{
 ::gvar=(1);
"Caught:GetDeclaration::InstanceDeclarationNotFound";
}

TopLevel::TopLevel(char (*name_2)) : add_inst("add_inst"), mul_inst("mul_inst")
{
}

void (cease)(void p1_1);

void (exit)(void p1_2);

void (next_trigger)(void p1_3);

void (wait)(void p1_4);

void (SC_METHOD)(void func);
