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
int j;
TopLevel top_level("top_level");
int i;
int gvar;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
if(  ::i<(4) )
{
NEXT:;
{
{
 ::gvar+= ::i;
{
 ::j=(0);
if(  ::j<(3) )
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE1:;
if(  ::j<(3) )
goto NEXT1;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE2:;
 ::i++;
}
CONTINUE3:;
if(  ::i<(4) )
goto NEXT;
}
}
cease(  ::gvar );
return ;
}
