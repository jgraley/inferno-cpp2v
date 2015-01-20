#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
void *link;
public:
/*temp*/ void *otherhelper_link;
void T();
/*temp*/ int helper_n;
void (otherhelper)();
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link_1;
/*temp*/ void *helper_link_1;
void (helper)();
};
int gvar;
int i;
int j;
TopLevel top_level("top_level");

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
 ::TopLevel::helper_link_1=(&&LINK);
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

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
{
temp_link= ::TopLevel::link;
return ;
}
}

void (TopLevel::helper)()
{
/*temp*/ void *temp_link;
auto int n;
auto void *link_1;
link_1= ::TopLevel::helper_link_1;
n= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<n;  ::j=( ::j+(1)) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
{
{
 ::TopLevel::otherhelper_link_1=(&&LINK);
 ::TopLevel::otherhelper();
}
LINK:;
}
{
temp_link=link_1;
return ;
}
}
