#include "isystemc.h"

class TopLevel;
int i;
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
void *link;
public:
/*temp*/ void *helper_link;
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link1;
/*temp*/ int helper_n;
void T();
/*temp*/ void *otherhelper_link1;
private:
void *(link_stack[10U]);
unsigned int helper_stack_index;
};
TopLevel top_level("top_level");
int j;
int gvar;

void TopLevel::T()
{
/*temp*/ int temp_n;
/*temp*/ void *temp_link;
/*temp*/ void *temp_link1;
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link1=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=( ::gvar*(2));
{
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
}
 ::i=( ::i+(1));
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
 ::gvar=((1)+ ::gvar);
 ::j=((1)+ ::j);
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
PROCEED_THEN_ELSE1:;
 ::TopLevel::otherhelper_link=(&&LINK1);
goto ENTER_otherhelper;
LINK1:;
temp_link1=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link1);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
