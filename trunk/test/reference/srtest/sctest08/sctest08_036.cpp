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
/*temp*/ int helper_n;
/*temp*/ void *helper_link;
void T();
/*temp*/ void *otherhelper_link;
/*temp*/ void *helper_link1;
private:
int (n_stack[10U]);
void *link;
unsigned int helper_stack_index;
public:
/*temp*/ void *otherhelper_link1;
private:
void *(link_stack[10U]);
};
int j;
int i;
int gvar;
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int temp_n;
/*temp*/ void *temp_link1;
 ::gvar=(1);
 ::i=(0);
goto *((!( ::i<(4))) ? (&&PROCEED_THEN_ELSE) : (&&PROCEED_NEXT));
PROCEED_NEXT:;
 ::gvar=( ::gvar+ ::i);
temp_n=(3);
 ::TopLevel::helper_n=temp_n;
 ::TopLevel::helper_link=(&&LINK);
goto ENTER_helper;
LINK:;
 ::gvar=( ::gvar*(2));
wait(SC_ZERO_TIME);
 ::i=( ::i+(1));
goto *(( ::i<(4)) ? (&&PROCEED_NEXT) : (&&PROCEED_THEN_ELSE));
goto PROCEED_THEN_ELSE;
PROCEED_THEN_ELSE:;
cease(  ::gvar );
return ;
ENTER_helper:;
 ::TopLevel::helper_stack_index++;
( ::TopLevel::link_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_link;
( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])= ::TopLevel::helper_n;
 ::j=(0);
goto *((!( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index]))) ? (&&PROCEED_THEN_ELSE1) : (&&PROCEED_NEXT1));
PROCEED_NEXT1:;
wait(SC_ZERO_TIME);
 ::gvar=( ::gvar+(1));
 ::j=( ::j+(1));
goto *(( ::j<( ::TopLevel::n_stack[ ::TopLevel::helper_stack_index])) ? (&&PROCEED_NEXT1) : (&&PROCEED_THEN_ELSE1));
goto PROCEED_THEN_ELSE1;
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