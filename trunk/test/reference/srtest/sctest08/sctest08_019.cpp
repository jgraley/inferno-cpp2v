#include "isystemc.h"

class TopLevel;
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
void *(link_stack[10U]);
public:
void T();
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link_1;
private:
void *link;
unsigned int helper_stack_index;
public:
/*temp*/ int helper_n;
};
TopLevel top_level("top_level");
int i;
int j;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
if(  ::i<(4) )
do
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
 ::i=((1)+ ::i);
}
while(  ::i<(4) );
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
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
do
{
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
CONTINUE_1:;
 ::j=( ::j+(1));
}
while(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) );
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
