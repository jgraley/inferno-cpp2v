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
void T();
void (helper)(auto int n);
void (otherhelper)();
};
TopLevel top_level("top_level");
int j;
int i;

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=( ::i+(1)) )
{
 ::gvar=( ::gvar+ ::i);
 ::TopLevel::helper(3);
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
}

void (TopLevel::helper)(int n)
{
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
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