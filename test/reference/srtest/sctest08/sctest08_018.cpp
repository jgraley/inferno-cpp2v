#include "isystemc.h"

class TopLevel;
int j;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
private:
int (n_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
private:
void *(link_stack[10U]);
public:
/*temp*/ int helper_n;
private:
unsigned int helper_stack_index;
public:
void T();
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link_1;
private:
void *link;
};
TopLevel top_level("top_level");
int i;

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
 ::TopLevel::helper_link_1=(&&LINK);
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
while(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE_1:;
 ::j=( ::j+(1));
}
}
{
{
 ::TopLevel::otherhelper_link_1=(&&LINK_1);
goto ENTER_otherhelper;
}
LINK_1:;
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
/*temp*/ void *temp_link_1;
 ::TopLevel::link= ::TopLevel::otherhelper_link_1;
 ::gvar=( ::gvar-(1));
{
temp_link_1= ::TopLevel::link;
goto *(temp_link_1);
}
}
}
