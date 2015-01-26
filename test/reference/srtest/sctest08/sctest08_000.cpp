#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (helper)(auto int n);
void T();
void (otherhelper)();
};
int j;
int i;
TopLevel top_level("top_level");
int gvar;

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

void (TopLevel::otherhelper)()
{
 ::gvar=( ::gvar-(1));
}
