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
};
TopLevel top_level("top_level");
void SC_ZERO_TIME;
int gvar;
int i;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);

void TopLevel::T()
{
 ::gvar=(1);
auto int t = 5;
for(  ::i=(0);  ::i<t;  ::i++ )
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::gvar*=(2);
}
cease(  ::gvar );
}

TopLevel::TopLevel(char (*name))
{
}

void (cease)(void p1);

void (exit)(void p1_1);

void (next_trigger)(void p1_2);

void (wait)(void p1_3);
