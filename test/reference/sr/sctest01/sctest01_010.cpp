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
sc_event proceed;
Adder(auto char (*name));
void (T)();
};
class Multiplier : public sc_module
{
public:
SC_CTOR( Multiplier )
{
}
sc_event instigate;
sc_event proceed_1;
Multiplier(auto char (*name_1));
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
void (cease)(void p1_1);
void (exit)(void p1_2);
void (next_trigger)(void p1_3);
void (wait)(void p1_4);
void (SC_METHOD)(void func_1);
void (SC_THREAD)(void func);
void (SC_CTHREAD)(void clock, void func_2);

Adder::Adder(char (*name))
{
 ::SC_THREAD( ::Adder::T);
}

void (Adder::T)()
{
wait(  ::Adder::proceed );
 ::gvar+=(2);
"ExtractInits() caught HasDeclaration::DeclarationNotFound";
wait(  ::Adder::proceed );
 ::gvar+=(3);
"ExtractInits() caught HasDeclaration::DeclarationNotFound";
}

Multiplier::Multiplier(char (*name_1))
{
 ::SC_THREAD( ::Multiplier::T_1);
}

void (Multiplier::T_1)()
{
wait(  ::Multiplier::instigate );
 ::gvar*=(5);
"ExtractInits() caught HasDeclaration::DeclarationNotFound";
wait(  ::Multiplier::proceed_1 );
 ::gvar*=(5);
"ExtractInits() caught HasDeclaration::DeclarationNotFound";
wait(  ::Multiplier::proceed_1 );
 ::cease( ::gvar);
}

TopLevel::TopLevel(char (*name_2)) : add_inst("add_inst"), mul_inst("mul_inst")
{
 ::SC_THREAD( ::TopLevel::T_2);
}

void (TopLevel::T_2)()
{
 ::gvar=(1);
"ExtractInits() caught HasDeclaration::DeclarationNotFound";
}

void (cease)(void p1_1);

void (exit)(void p1_2);

void (next_trigger)(void p1_3);

void (wait)(void p1_4);

void (SC_METHOD)(void func_1);

void (SC_THREAD)(void func);

void (SC_CTHREAD)(void clock, void func_2);
