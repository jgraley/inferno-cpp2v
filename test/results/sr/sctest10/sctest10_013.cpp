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
int (f)(auto char k, auto short j, auto int i);
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);
void (SC_METHOD)(void func);

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f(8, 6,  ::TopLevel::f(0, 0, 0)));
cease(  ::TopLevel::x+((2)*( ::TopLevel::f(3, 2, 1))) );
}

TopLevel::TopLevel(char (*name))
{
}

int (TopLevel::f)(char k, short j, int i)
{
auto int t = i+((3)*j);
return t+((5)*k);
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);

void (SC_METHOD)(void func);
