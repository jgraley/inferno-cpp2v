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
TopLevel top_level("top_level");
int gvar;
int i;
int j;

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
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
CONTINUE:;
 ::j++;
}
CONTINUE_1:;
if(  ::j<(3) )
goto NEXT_1;
}
}
 ::gvar*=(2);
wait(SC_ZERO_TIME);
}
CONTINUE_2:;
 ::i++;
}
CONTINUE_3:;
if(  ::i<(4) )
goto NEXT;
}
}
cease(  ::gvar );
return ;
}
