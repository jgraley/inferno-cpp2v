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
int j;
void (cease)(void p1);
void (exit)(void p1_1);
void (next_trigger)(void p1_2);
void (wait)(void p1_3);

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i++ )
{
 ::gvar+= ::i;
for(  ::j=(0);  ::j<(3);  ::j++ )
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
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
