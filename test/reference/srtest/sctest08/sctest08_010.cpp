#include "isystemc.h"

class TopLevel;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *otherhelper_link;
void (otherhelper)(auto void *link);
void T();
/*temp*/ void *helper_link;
void (helper)(auto void *link, auto int n);
};
TopLevel top_level("top_level");
int i;
int gvar;

void (TopLevel::otherhelper)(void *link)
{
/*temp*/ void *temp_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=( ::i+(1)) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
 ::TopLevel::helper(&&LINK, temp_n);
LINK:;
}
});
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}

void (TopLevel::helper)(void *link, int n)
{
/*temp*/ void *temp_link;
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
temp_link=link;
return ;
}
}
