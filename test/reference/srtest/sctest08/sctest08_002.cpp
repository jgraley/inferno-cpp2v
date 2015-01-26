#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (otherhelper)();
void T();
void (helper)(auto int n);
};
int j;
TopLevel top_level("top_level");
int gvar;
int i;

void (TopLevel::otherhelper)()
{
 ::gvar=( ::gvar-(1));
}

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=( ::i+(1)) )
{
 ::gvar=( ::gvar+ ::i);
 ::TopLevel::helper(3);
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
}

void (TopLevel::helper)(int n)
{
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
 ::TopLevel::otherhelper();
return ;
}
