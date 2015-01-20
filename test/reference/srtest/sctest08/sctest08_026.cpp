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
/*temp*/ void *helper_link;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *helper_link_1;
/*temp*/ void *otherhelper_link;
private:
unsigned int helper_stack_index;
int (n_stack[10U]);
public:
/*temp*/ int helper_n;
void T();
/*temp*/ void *otherhelper_link_1;
private:
void *link;
};
TopLevel top_level("top_level");
int j;

void TopLevel::T()
{
 ::gvar=(1);
{
 ::i=(0);
{
if( !( ::i<(4)) )
goto THEN;
{
NEXT:;
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
CONTINUE_1:;
if(  ::i<(4) )
goto NEXT;
}
goto ELSE;
THEN:;
;
ELSE:;
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
{
if( !( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) )
goto THEN_1;
{
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE_2:;
 ::j=( ::j+(1));
}
CONTINUE_3:;
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
goto NEXT_1;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
}
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
