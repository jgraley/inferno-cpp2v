#include "isystemc.h"

class TopLevel;
int j;
int gvar;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *otherhelper_link;
void (otherhelper)();
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link_1;
void (helper)();
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
void T();
};
TopLevel top_level("top_level");

void (TopLevel::otherhelper)()
{
auto void *link;
/*temp*/ void *temp_link;
link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}

void (TopLevel::helper)()
{
auto int n;
/*temp*/ void *temp_link;
auto void *link;
link= ::TopLevel::helper_link;
n= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
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
 ::TopLevel::helper_link=(&&LINK);
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
