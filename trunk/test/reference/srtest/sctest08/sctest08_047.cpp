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
private:
void *(link_stack[10U]);
public:
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link;
private:
int (n_stack[10U]);
void *link;
public:
/*temp*/ int helper_n;
private:
unsigned int helper_stack_index;
public:
/*temp*/ void *helper_link1;
/*temp*/ void *otherhelper_link1;
void T();
};
int i;
int j;
TopLevel top_level("top_level");
int gvar;

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ void *temp_link1;
/*temp*/ int temp_n;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link1=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=((2)* ::gvar);
{
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
}
 ::i=((1)+ ::i);
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
goto ENTER_helper;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
{
wait(SC_ZERO_TIME);
goto YIELD1;
YIELD1:;
}
 ::gvar=( ::gvar+(1));
 ::j=((1)+ ::j);
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link=(&&LINK1);
goto ENTER_otherhelper;
LINK1:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link1= ::TopLevel::link;
goto *(temp_link1);
}