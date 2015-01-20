#include "isystemc.h"

class TopLevel;
int gvar;
int i;
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
if(  ::i<(5) )
{
NEXT:;
{
{
 ::gvar+= ::i;
if( ( ::i%(2))==(0) )
{
wait(SC_ZERO_TIME);
 ::gvar^=(1);
}
 ::gvar*=(2);
}
CONTINUE:;
 ::i++;
}
CONTINUE_1:;
if(  ::i<(5) )
goto NEXT;
}
}
cease(  ::gvar );
return ;
}
