#include "isystemc.h"

class TopLevel;
int gvar;
int i;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
/*temp*/ void *otherhelper_link;
private:
void *link;
int (n_stack[10U]);
unsigned int helper_stack_index;
public:
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link1;
/*temp*/ int helper_n;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link1;
void T();
};
TopLevel top_level("top_level");
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
 ::TopLevel::helper_link1=(&&LINK);
goto ENTER_helper;
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
ENTER_helper:;
{
{
/*temp*/ void *temp_link;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
for(  ::j=(0);  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]);  ::j=((1)+ ::j) )
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
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
