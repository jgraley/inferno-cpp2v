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
for(  ::i=(0);  ::i<(4);  ::i++ )
{
 ::gvar+= ::i;
for(  ::j=(0);  ::j<(3);  ::j++ )
{
wait(SC_ZERO_TIME);
 ::gvar++;
}
 ::gvar*=(2);
}
cease(  ::gvar );
return ;
}