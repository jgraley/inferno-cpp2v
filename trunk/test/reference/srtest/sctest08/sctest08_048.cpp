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
/*temp*/ void *otherhelper_link;
private:
void *link;
int (n_stack[10U]);
unsigned int helper_stack_index;
public:
/*temp*/ void *helper_link;
/*temp*/ void *helper_link_1;
void T();
/*temp*/ int helper_n;
/*temp*/ void *otherhelper_link_1;
private:
void *(link_stack[10U]);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int temp_n;
/*temp*/ void *temp_link_1;
 ::gvar=(1);
 ::i=(0);
wait(SC_ZERO_TIME);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link_1=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=((2)* ::gvar);
wait(SC_ZERO_TIME);
goto YIELD;
YIELD:;
 ::i=( ::i+(1));
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
goto ENTER_helper;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link_1;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&PROCEED_THEN_ELSE_1) : (&&PROCEED_NEXT_1));
PROCEED_NEXT_1:;
wait(SC_ZERO_TIME);
goto YIELD_1;
YIELD_1:;
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT_1) : (&&PROCEED_THEN_ELSE_1));
PROCEED_THEN_ELSE_1:;
 ::TopLevel::otherhelper_link=(&&LINK_1);
goto ENTER_otherhelper;
LINK_1:;
temp_link=( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index]);
 ::TopLevel::helper_stack_index--;
goto *(temp_link);
ENTER_otherhelper:;
 ::TopLevel::link= ::TopLevel::otherhelper_link;
 ::gvar=( ::gvar-(1));
temp_link_1= ::TopLevel::link;
goto *(temp_link_1);
}
