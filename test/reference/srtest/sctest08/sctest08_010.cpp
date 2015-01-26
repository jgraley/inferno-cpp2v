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
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
void (otherhelper)(auto void *link);
void (helper)(auto int n, auto void *link);
void T();
};
TopLevel top_level("top_level");
int j;

void (TopLevel::otherhelper)(void *link)
{
/*temp*/ void *temp_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}

void (TopLevel::helper)(int n, void *link)
{
/*temp*/ void *temp_link;
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
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

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=((1)+ ::i) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
 ::TopLevel::helper(temp_n, &&LINK);
LINK:;
}
});
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}
