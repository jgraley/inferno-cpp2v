#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
}
TopLevel(auto char (*name));
int x;
void (T)();
int (f)(auto int i);
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);
void (SC_METHOD)(void func_1);
void (SC_THREAD)(void func);
void (SC_CTHREAD)(void clock, void func_2);

TopLevel::TopLevel(char (*name))
{
 ::SC_THREAD( ::TopLevel::T);
}

void (TopLevel::T)()
{
 ::cease( ::TopLevel::f(1));
}

int (TopLevel::f)(int i)
{
return i;
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);

void (SC_METHOD)(void func_1);

void (SC_THREAD)(void func);

void (SC_CTHREAD)(void clock, void func_2);
