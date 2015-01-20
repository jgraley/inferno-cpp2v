#include "isystemc.h"

class TopLevel;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (otherhelper)();
void (helper)(auto int n);
void T();
};
TopLevel top_level("top_level");
int j;
int i;

void (TopLevel::otherhelper)()
{
 ::gvar=( ::gvar-(1));
}

void (TopLevel::helper)(int n)
{
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
 ::TopLevel::otherhelper();
return ;
}

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=((1)+ ::i) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3);  ::TopLevel::helper(temp_n); });
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
}
