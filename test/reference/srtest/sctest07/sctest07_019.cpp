#include "isystemc.h"

class TopLevel;
int gvar;
int i;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
if(  ::i<(4) )
do
{
{
 ::gvar+= ::i;
{
 ::j=(0);
if(  ::j<(3) )
do
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
while(  ::j<(3) );
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE1:;
 ::i++;
}
while(  ::i<(4) );
}
cease(  ::gvar );
return ;
}