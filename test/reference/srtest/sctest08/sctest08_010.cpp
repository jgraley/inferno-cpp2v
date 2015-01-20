#include "isystemc.h"

class TopLevel;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
/*temp*/ void *helper_link;
void (helper)(auto int n, auto void *link);
void (otherhelper)(auto void *link);
/*temp*/ void *otherhelper_link;
};
int gvar;
int j;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
for(  ::i=(0);  ::i<(4);  ::i=( ::i+(1)) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
 ::TopLevel::helper(temp_n, &&LINK);
LINK:;
}
});
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
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

void (TopLevel::otherhelper)(void *link)
{
/*temp*/ void *temp_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}
