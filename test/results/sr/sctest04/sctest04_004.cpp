#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
}
TopLevel(auto char (*name));
void (T)();
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
int gvar;
int i;
void (cease)(void p1_1);
void (exit)(void p1_2);
void (next_trigger)(void p1_3);
void (wait)(void p1);
void (SC_METHOD)(void func_1);
void (SC_THREAD)(void func);
void (SC_CTHREAD)(void clock, void func_2);

TopLevel::TopLevel(char (*name))
{
 ::SC_THREAD( ::TopLevel::T);
}

void (TopLevel::T)()
{
 ::gvar=(1);
auto int t = 5;
for(  ::i=(0);  ::i<t;  ::i++ )
{
 ::gvar+= ::i;
 ::wait( ::SC_ZERO_TIME);
 ::gvar*=(2);
}
 ::cease( ::gvar);
}

void (cease)(void p1_1);

void (exit)(void p1_2);

void (next_trigger)(void p1_3);

void (wait)(void p1);

void (SC_METHOD)(void func_1);

void (SC_THREAD)(void func);

void (SC_CTHREAD)(void clock, void func_2);