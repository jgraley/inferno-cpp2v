#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0)
{
SC_THREAD(T);
}
void T();
private:
int (n_stack[10]);
void *(link_stack[10]);
void *link;
unsigned int helper_stack_index;
public:
/*temp*/ int helper_n;
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link_1;
};
TopLevel top_level("top_level");
int gvar;
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
 ::TopLevel::helper_link=(&&LINK);
{
 ::TopLevel::helper_n=temp_n;
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
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
{
 ::j=(0);
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
do
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE_1:;
 ::j=((1)+ ::j);
}
while(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) );
}
{
{
 ::TopLevel::otherhelper_link=(&&LINK_1);
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
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
{
temp_link_1= ::TopLevel::link;
goto *(temp_link_1);
}
}
}