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
int i;
TopLevel top_level("top_level");
int gvar;

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
wait(SC_ZERO_TIME);
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
