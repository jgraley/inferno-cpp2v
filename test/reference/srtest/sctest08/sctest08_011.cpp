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
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link1;
void (helper)();
void T();
/*temp*/ void *otherhelper_link1;
/*temp*/ int helper_n;
};
int i;
TopLevel top_level("top_level");
int j;
int gvar;

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}

void (TopLevel::helper)()
{
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::helper_link1;
auto int n =  ::TopLevel::helper_n;
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK);
 ::TopLevel::otherhelper();
}
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
for(  ::i=(0);  ::i<(4);  ::i=( ::i+(1)) )
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
{
 ::TopLevel::helper_n=temp_n;
{
 ::TopLevel::helper_link1=(&&LINK);
 ::TopLevel::helper();
}
}
LINK:;
}
});
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}
