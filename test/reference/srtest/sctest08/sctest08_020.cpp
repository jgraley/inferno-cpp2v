#include "isystemc.h"

class TopLevel;
int gvar;
int i;
int j;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel ) :
helper_stack_index(0U)
{
SC_THREAD(T);
}
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
/*temp*/ void *otherhelper_link1;
/*temp*/ int helper_n;
/*temp*/ void *helper_link1;
private:
void *link;
unsigned int helper_stack_index;
int (n_stack[10U]);
void *(link_stack[10U]);
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
if(  ::i<(4) )
{
NEXT:;
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
 ::i=( ::i+(1));
}
CONTINUE1:;
if(  ::i<(4) )
goto NEXT;
}
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
{
NEXT1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
}
CONTINUE2:;
 ::j=( ::j+(1));
}
CONTINUE3:;
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
goto NEXT1;
}
}
{
{
 ::TopLevel::otherhelper_link1=(&&LINK1);
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
 ::TopLevel::link= ::TopLevel::otherhelper_link1;
 ::gvar=( ::gvar-(1));
{
temp_link1= ::TopLevel::link;
goto *(temp_link1);
}
}
}