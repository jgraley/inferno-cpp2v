#include "isystemc.h"

class TopLevel;
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
int gvar;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
while(  ::i<(5) )
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
}
cease(  ::gvar );
return ;
}
