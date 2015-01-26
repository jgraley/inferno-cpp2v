#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
/*temp*/ int helper_n;
void T();
private:
void *link;
public:
/*temp*/ void *otherhelper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link1;
private:
int (n_stack[10U]);
void *(link_stack[10U]);
};
int j;
int gvar;
int i;
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
while(  ::i<(4) )
{
{
 ::gvar=( ::gvar+ ::i);
({ /*temp*/ int temp_n; temp_n=(3); {
{
 ::TopLevel::helper_n=temp_n;
{
 ::TopLevel::helper_link=(&&LINK);
goto ENTER_helper;
}
}
LINK:;
}
});
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
}
CONTINUE:;
 ::i=( ::i+(1));
}
}
cease(  ::gvar );
return ;
ENTER_helper:;
{
{
/*temp*/ void *temp_link;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
while(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE1:;
 ::j=((1)+ ::j);
}
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK1);
goto ENTER_otherhelper;
}
LINK1:;
}
{
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
{
 ::TopLevel::helper_stack_index--;
goto *(temp_link);
}
}
}
}
ENTER_otherhelper:;
{
/*temp*/ void *temp_link1;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link1= ::TopLevel::link;
goto *(temp_link1);
}
}
}
