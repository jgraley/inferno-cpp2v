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
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
while(  ::i<(5) )
{
{
 ::gvar+= ::i;
if( (0)==( ::i%(2)) )
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
