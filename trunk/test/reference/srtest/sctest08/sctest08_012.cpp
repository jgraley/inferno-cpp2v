#include "isystemc.h"

class TopLevel;
int j;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
void (helper)();
/*temp*/ void *otherhelper_link;
void T();
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link1;
void (otherhelper)();
};
TopLevel top_level("top_level");
int gvar;

void (TopLevel::helper)()
{
auto int n;
auto void *link;
/*temp*/ void *temp_link;
link= ::TopLevel::helper_link;
n= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
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

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link;
auto void *link;
link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link=link;
return ;
}
}
