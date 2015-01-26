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
void (helper)(auto int n);
void (otherhelper)();
void T();
};
int j;
TopLevel top_level("top_level");

void (TopLevel::helper)(int n)
{
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
 ::TopLevel::otherhelper();
return ;
}

void (TopLevel::otherhelper)()
{
 ::gvar=( ::gvar-(1));
}

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=((1)+ ::i) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3);  ::TopLevel::helper(temp_n); });
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
}
