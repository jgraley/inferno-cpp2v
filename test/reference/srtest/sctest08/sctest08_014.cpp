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
private:
void *link;
public:
/*temp*/ void *helper_link;
void (helper)();
/*temp*/ void *helper_link1;
void T();
/*temp*/ void *otherhelper_link;
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link1;
};
int gvar;
TopLevel top_level("top_level");
int j;
int i;

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link= ::TopLevel::link;
return ;
}
}

void (TopLevel::helper)()
{
auto int n;
/*temp*/ void *temp_link;
auto void *link1;
link1= ::TopLevel::helper_link;
n= ::TopLevel::helper_n;
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
temp_link=link1;
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
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}
