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
void (helper)(auto void *link1, auto int n);
void (otherhelper)(auto void *link);
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=((1)+ ::i) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
 ::TopLevel::helper(&&LINK1, temp_n);
LINK1:;
}
});
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}

void (TopLevel::helper)(void *link1, int n)
{
/*temp*/ void *temp_link1;
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
{
 ::TopLevel::otherhelper(&&LINK);
LINK:;
}
{
temp_link1=link1;
return ;
}
}

void (TopLevel::otherhelper)(void *link)
{
/*temp*/ void *temp_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}
