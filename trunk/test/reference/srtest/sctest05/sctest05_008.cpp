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
for(  ::i=(0);  ::i<(5);  ::i++ )
{
 ::gvar+= ::i;
if( ( ::i%(2))==(0) )
{
wait(SC_ZERO_TIME);
 ::gvar^=(1);
}
 ::gvar*=(2);
}
cease(  ::gvar );
return ;
}
