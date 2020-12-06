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
void (helper)();
void (otherhelper)();
/*temp*/ int helper_n;
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link1;
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
/*temp*/ void *temp_link;
auto void *link =  ::TopLevel::helper_link;
auto int n =  ::TopLevel::helper_n;
for(  ::j=(0);  ::j<n;  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK1);
 ::TopLevel::otherhelper();
}
LINK1:;
}
{
temp_link=link;
return ;
}
}

void (TopLevel::otherhelper)()
{
/*temp*/ void *temp_link1;
auto void *link1 =  ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link1=link1;
return ;
}
}
