#include "isystemc.h"

class TopLevel;
int i;
int gvar;
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
for(  ::i=(0);  ::i<(5);  ::i++ )
{
 ::gvar+= ::i;
wait(SC_ZERO_TIME);
 ::gvar*=(2);
}
cease(  ::gvar );
return ;
}
