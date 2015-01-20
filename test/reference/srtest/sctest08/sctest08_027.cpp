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
void T();
/*temp*/ void *helper_link;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *helper_link_1;
private:
int (n_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
private:
void *(link_stack[10U]);
public:
/*temp*/ void *otherhelper_link_1;
private:
void *link;
public:
/*temp*/ int helper_n;
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
goto PROCEED;
PROCEED:;
{
NEXT:;
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
CONTINUE_1:;
if(  ::i<(4) )
goto NEXT;
goto PROCEED_1;
PROCEED_1:;
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
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
{
 ::j=(0);
{
if( !( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) )
goto THEN_1;
goto PROCEED_2;
PROCEED_2:;
{
NEXT_1:;
{
{
wait(SC_ZERO_TIME);
 ::gvar=((1)+ ::gvar);
}
CONTINUE_2:;
 ::j=((1)+ ::j);
}
CONTINUE_3:;
if(  ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]) )
goto NEXT_1;
goto PROCEED_3;
PROCEED_3:;
}
goto ELSE_1;
THEN_1:;
;
ELSE_1:;
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
