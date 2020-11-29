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
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
while(  ::i<(4) )
{
{
 ::gvar+= ::i;
{
 ::j=(0);
while(  ::j<(3) )
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE1:;
 ::i++;
}
}
cease(  ::gvar );
return ;
}
