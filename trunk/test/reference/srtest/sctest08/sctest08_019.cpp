#include "isystemc.h"

class TopLevel;
int i;
int gvar;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
void T();
/*temp*/ int helper_n;
private:
int (n_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link1;
/*temp*/ void *helper_link;
/*temp*/ void *helper_link1;
private:
void *link;
void *(link_stack[10U]);
};
TopLevel top_level("top_level");
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
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
CONTINUE1:;
 ::j=( ::j+(1));
}
while(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) );
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