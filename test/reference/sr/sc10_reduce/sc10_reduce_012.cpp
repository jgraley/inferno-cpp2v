#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
TopLevel(auto char (*name));
int x;
int (f)(auto int i);
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);
void (SC_METHOD)(void func);
void (SC_CTHREAD)(void clock, void func_1);

void TopLevel::T()
{
cease(  ::TopLevel::f(1) );
}

TopLevel::TopLevel(char (*name))
{
}

int (TopLevel::f)(int i)
{
return i;
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);

void (SC_METHOD)(void func);

void (SC_CTHREAD)(void clock, void func_1);
