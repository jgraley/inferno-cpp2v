#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *helper_link;
void T();
/*temp*/ void *otherhelper_link;
void (helper)();
/*temp*/ void *helper_link1;
/*temp*/ int helper_n;
private:
void *link;
public:
/*temp*/ void *otherhelper_link1;
void (otherhelper)();
};
TopLevel top_level("top_level");
int gvar;
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
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
}
cease(  ::gvar );
return ;
}

void (TopLevel::helper)()
{
auto int n;
auto void *link1;
/*temp*/ void *temp_link;
link1= ::TopLevel::helper_link;
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
temp_link=link1;
return ;
}
}

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
