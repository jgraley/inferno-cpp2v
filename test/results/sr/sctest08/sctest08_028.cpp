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
void (otherhelper)(auto void *link_1);
void (helper)(auto int n, auto void *link);
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
};
TopLevel top_level("top_level");
int gvar;
int i;
int j;

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

void (TopLevel::otherhelper)(void *link_1)
{
/*temp*/ void *temp_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link_1;
return ;
}
}

void (TopLevel::helper)(int n, void *link)
{
/*temp*/ void *temp_link_1;
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
{
 ::TopLevel::otherhelper(&&LINK_1);
LINK_1:;
}
{
temp_link_1=link;
return ;
}
}
