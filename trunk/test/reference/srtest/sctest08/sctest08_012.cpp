#include "isystemc.h"

class TopLevel;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
void (helper)();
/*temp*/ void *helper_link1;
/*temp*/ void *otherhelper_link1;
/*temp*/ int helper_n;
void (otherhelper)();
};
TopLevel top_level("top_level");
int j;
int i;

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
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
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
 ::gvar=((1)+ ::gvar);
}
{
{
 ::TopLevel::otherhelper_link1=(&&LINK);
 ::TopLevel::otherhelper();
}
LINK:;
}
{
temp_link=link;
return ;
}
}

void (TopLevel::otherhelper)()
{
auto void *link;
/*temp*/ void *temp_link;
link= ::TopLevel::otherhelper_link1;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}
